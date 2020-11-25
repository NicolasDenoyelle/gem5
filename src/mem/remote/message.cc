#include "message.hh"

template <class T>
bool Array<T>::operator==(const Array<T>& other){
    uint64_t n = _messages.size();
    if (n != other._messages.size())
        return false;
    for (uint64_t i = 0; i < n; i++)
        if (_messages[i] != other._messages[i])
            return false;
    return true;
}

template <class T>
uint64_t Array<T>::len() const{
    uint64_t tot = 17;
    for (auto msg = _messages.cbegin(); msg != _messages.cend(); msg++)
        tot += msg->len();
    return tot;
}

template <class T>
void Array<T>::copyBytes(void* out) const{
    *(uint8_t *) out = uid();
    *(uint64_t *) PTR_OFF(out, 1) = len();
    *(uint64_t *) PTR_OFF(out, 9) = _messages.size();
    out = PTR_OFF(out, 17);
    for (auto msg = _messages.cbegin(); msg != _messages.cend(); msg++) {
        msg->copyBytes(out);
        out = PTR_OFF(out, msg->len());
    }
}

template <class T>
void Array<T>::fromBytes(const void *bytes){
    uint64_t n = *(uint64_t *)PTR_OFF(bytes, 9);
    _messages.clear();
    bytes = PTR_OFF(bytes, 17);
    for (uint64_t i = 0; i < n; i++) {
        T msg;
        msg.fromBytes(bytes);
        _messages.push_back(msg);
        bytes = PTR_OFF(bytes, _messages.back().len());
    }
}

template class Array<MemAccess>;
