#include "RunExecutable.h"
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

using namespace System;
using namespace Events;


namespace {

    struct EventImpl final : Event<char>::Concept {
        std::string m_exe;
        std::vector<std::string> m_args;
        static constexpr int PIPE_READ_END=0;
        static constexpr int PIPE_WRITE_END=1;

        EventImpl(std::string exe, std::vector<std::string> args)
            : m_exe{std::move(exe)}
            , m_args{std::move(args)}
        {}

        Canceler subscribe(Listener<char> listener) override {
            int fd[2];
            if (pipe (fd))
                throw std::error_code(errno, std::system_category());

            if(auto pid = fork()) {
                continueInParentProcess(fd, std::move(pid), std::move(listener));
            }
            else {
                continueInChildProcess(fd);
            }

            return { std::make_unique<Canceler::Concept>() };
        };

    private:
        void continueInParentProcess(int fd[2], pid_t pid, Listener<char> listener)
        {
            close(fd[PIPE_WRITE_END]);

            int c;
            std::shared_ptr<FILE> stream(fdopen (fd[PIPE_READ_END], "r"), [](auto p){ fclose(p); });

            while ((c = fgetc (stream.get())) != EOF){
                listener.notify(c);
            }

            stream = nullptr;

            int returnStatus;
            waitpid(pid, &returnStatus, 0);
            if(returnStatus)
                throw std::error_code(returnStatus, std::system_category());
        }

        void continueInChildProcess(int fd[2])
        {
            dup2(fd[PIPE_WRITE_END], STDOUT_FILENO);
            dup2(fd[PIPE_WRITE_END], STDERR_FILENO);
            close(fd[PIPE_READ_END]);
            close(fd[PIPE_WRITE_END]);

            auto argvp = new char const*[m_args.size() + 2u];
            argvp[0] = m_exe.c_str();

            for(std::size_t i = 0; i < m_args.size(); ++i) {
                argvp[i+1u] = m_args[i].c_str();
            }

            argvp[m_args.size()+1u] = (char const*)nullptr;

            // child process
            execvp(m_exe.c_str(), (char* const*) argvp);

            // no return unless error
            throw std::error_code(errno, std::system_category());
        }
    };

}


Event<char> System::runExecutable(std::string exe, std::vector<std::string> args) {
    return { std::make_unique<EventImpl>(std::move(exe), std::move(args)) };
}
