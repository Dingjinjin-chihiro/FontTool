// Reused from PoDoFo auxiliary/StateStack.h; throwing Pop replaced by bool.
// SPDX-License-Identifier: LGPL-2.0-or-later OR MPL-2.0
#pragma once
#include <stack>
namespace PoDoFo {
template <typename StateT> class StateStack final {
    struct Accessor final { friend class StateStack; private: Accessor() noexcept:m_state(nullptr){} void Set(StateT& s) noexcept{m_state=&s;} public: StateT* operator->() noexcept{return m_state;} const StateT* operator->() const noexcept{return m_state;} StateT& operator*() noexcept{return *m_state;} const StateT& operator*() const noexcept{return *m_state;} private: StateT* m_state; };
public:
    Accessor Current;
    StateStack(){push({});}
    void Push(){push(m_states.top());}
    bool PopLenient(unsigned count=1) noexcept{return Pop(count);}
    bool Pop(unsigned count=1) noexcept{if(count>=m_states.size())return false;while(count--)m_states.pop();Current.Set(m_states.top());return true;}
    void Clear(){while(!m_states.empty())m_states.pop();push({});}
    unsigned GetSize() const noexcept{return static_cast<unsigned>(m_states.size());}
private:
    void push(const StateT& state){m_states.push(state);Current.Set(m_states.top());}
    std::stack<StateT> m_states;
};
}
