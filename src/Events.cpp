#include "Events.h"

using namespace Events;

Canceler::~Canceler() {
    m_impl.reset();
}

void Canceler::cancel() {
    m_impl->cancel();
}
