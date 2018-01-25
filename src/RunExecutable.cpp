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
        static constexpr int PIPE_READ_END=0;
        static constexpr int PIPE_WRITE_END=1;

        EventImpl(std::string exe)
            : m_exe{std::move(exe)}
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

            // child process
            execlp(m_exe.c_str(), m_exe.c_str(), "--help", (char*)nullptr);
            // no return unless error
            throw std::error_code(errno, std::system_category());
        }
    };

}


Event<char> System::runExecutable(std::string exe) {
    return { std::make_unique<EventImpl>(std::move(exe)) };
}
