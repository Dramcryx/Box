# Box
## Hide your implementation without heap allocation

The idea comes from FastPimpl idiom which is used to allow forawrd-declaration of some implementation type of a class without completely defining it but specifying its' size and alignment.

FastPimpl is basically this:
```c++
template <typename T, std::size_t Size, std::size_t Alignemnt>
class FastPiml {
    template <typename ... Args>
    FastPimpl(Args&& ... args)
    {
        new (&m_storage) T{std::forward(args)...};
    }

    ~FastPimpl()
    {
        static_assert(sizeof(T) <= Size, ...);
        static_assert(alignof(T) % Alignemnt == 0, ...);
        Get()->~T();
    }

    T* Get()
    {
        return reinterpret_cast<T*>(&m_storage);
    }

private:
    alignas(Alignment) unsigned char m_storage[Size];
};
```

So the size is something known but not the actual type which is only chekced in the destructor. For all other cases, forward declaration of `T` is sufficient.

Now what if we use this idiom for this:
1) Create or use the interface which you typically return to a user as a `std::unique_ptr`;
2) Define size and alignment of that interface next to interface;
3) Wrap it into a type which offers access to the interface and holds the underlying object in the `unsigned char` buffer.

This is how `Box` works.

Consider a hierarchy like this:
```c++
// IIdentity.h
struct IIdentity {
    virtual ~IIdentity() = default;

    virtual void PrintWhoAmI() = 0;
};

// Identity.h
class Identity : public IIdentity {
// .. definition
};
```

Usually, when implementation does not allow user to know what is `Identity`, it is returned as `std::unique_ptr<IIdentity>` which is fine but causes any access to `IIdentity` methods to have two indirections instead of one - first for object and second for vtable.

With `Box` all you need is to add to `IIdentity.h` this:
```c++
#include "Box.h"

template <>
struct BoxSize<IIdentity> {
    static constexpr std::size_t Size = 8;
    static constexpr std::size_t Alignment = 8;
};
```
And then methods that previously returned `std::unique_ptr<IIdentity>` can now return `Box<IIdentity>`.

## Requirements
To be able to store the type in STL containers, all implementation types are required to be nothrow movable. Otherwise, we introduce even more runtime overhead.

## Overhead
Another overhead point is additional two pointer required:
1) Pointer to interface. Why? Because of multiple and diamond inheritance types. We cannot cast beginning of storage safely, so this pointer is cached at the moment of construction;
2) Pointer to move function. Since we lose original type info (i.e. interface implementation type), for the user code to be able to pass around the `Box` we need to be able to move it. This pointer is also cached in the constructor.
