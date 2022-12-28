#ifndef KERNEL_OBJECT_H
#define KERNEL_OBJECT_H

template <typename T>
class KernelObject {

public:
    KernelObject() = default;
    static void ctor(void* addr);
};

template<typename T>
void KernelObject<T>::ctor(void *addr) {
    T* obj = (T*) addr;
    T temp;
    *obj = temp;
}


#endif