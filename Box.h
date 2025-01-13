#pragma once

#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

namespace dramcryx {

// Basic requirement for boxable interface is to have a virtual destructor
//
template <typename TBoxed>
struct CanBeBoxed {
    static constexpr bool Value = std::has_virtual_destructor_v<TBoxed>;
};

// Requirements for a type that is being boxed as an interface
//
template <typename TBoxed, std::size_t TBoxedSize, std::size_t TBoxedAlignment, typename TUnboxed>
struct ValidBox {
    static constexpr bool NonNullSizes           = TBoxedSize > 0 && TBoxedAlignment > 0;
    static constexpr bool CorrectDeriviation     = std::is_base_of_v<TBoxed, TUnboxed>;
    static constexpr bool CastablePointers       = std::is_convertible_v<TUnboxed*, TBoxed*>;
    static constexpr bool HasVirtualDestructor   = std::has_virtual_destructor_v<TUnboxed>;
    static constexpr bool FitsSize               = sizeof(TUnboxed) >= sizeof(TBoxed);
    static constexpr bool FitsBox                = TBoxedSize >= sizeof(TUnboxed);
    static constexpr bool AlignedWithBox         = alignof(TUnboxed) % TBoxedAlignment == 0;
    static constexpr bool IsNoexceptMovable      = std::is_nothrow_move_constructible_v<TUnboxed>;
    static constexpr bool IsNoexceptDestructible = std::is_nothrow_destructible_v<TUnboxed>;

    static constexpr bool Value =
        NonNullSizes &&
        CorrectDeriviation &&
        CastablePointers &&
        HasVirtualDestructor &&
        FitsSize &&
        FitsBox &&
        AlignedWithBox &&
        IsNoexceptMovable &&
        IsNoexceptDestructible;
};

// Alias to value of box checking result
//
template <typename TBoxed, std::size_t TBoxedSize, std::size_t TBoxedAlignment, typename TUnboxed>
static constexpr bool ValidBoxV = ValidBox<TBoxed, TBoxedSize, TBoxedAlignment, TUnboxed>::Value;

// Tag type to allow inplace construction of boxed object
//
template <typename T>
struct BoxInplaceT {};

// Implementation of the box
//
template <typename TBoxed, std::size_t TBoxedSize, std::size_t TBoxedAlignment>
class BoxImpl
{
    static_assert(CanBeBoxed<TBoxed>::Value, "Cannot box TBoxed type");

    // Function type that is used to move boxes
    //
    using TBoxedMover = TBoxed*(void*, void*);

public:
    // Do not allow default constructors
    //
    BoxImpl() = delete;

    // Move constructor. Destroys current object if any,
    // and moves other one into current storage.
    //
    inline BoxImpl(BoxImpl&& other) noexcept
    {
        Destroy();

        m_cachedPointer = other.m_mover(&other.m_storage, &m_storage);
        m_mover = other.m_mover;
    }

    // Move-assignment operator. Destroys current object if any,
    // and moves other one into current storage.
    //
    inline BoxImpl& operator=(BoxImpl&& other) noexcept
    {
        Destroy();

        m_cachedPointer = other.m_mover(&other.m_storage, &m_storage);
        m_mover = other.m_mover;

        return *this;
    }

    // Inplace construction of a boxed object
    //
    template<typename TUnboxed, typename ... TArgs>
    inline BoxImpl(BoxInplaceT<TUnboxed>, TArgs&& ... args) :
        m_cachedPointer{new (&m_storage) TUnboxed{std::forward<TArgs>(args)...}},
        m_mover{Mover<TUnboxed>}
    {
        static_assert(
            ValidBoxV<TBoxed, TBoxedSize, TBoxedAlignment, TUnboxed>,
            "TUnobxed cannot be boxed into TBoxed");
    }

    // Construction from an object to be boxed
    //
    template<typename TUnboxed>
    inline BoxImpl(TUnboxed&& unboxedValue) :
        m_cachedPointer{new (&m_storage) TUnboxed{std::move(unboxedValue)}},
        m_mover{Mover<TUnboxed>}
    {
        static_assert(
            ValidBoxV<TBoxed, TBoxedSize, TBoxedAlignment, TUnboxed>,
            "TUnobxed cannot be boxed into TBoxed");
    }

    // Destructor. Just destroy held object.
    //
    inline ~BoxImpl() noexcept
    {
        Destroy();
    }

    // Assignment of a new object to be boxed
    //
    template<typename TUnboxed>
    inline BoxImpl& operator=(TUnboxed&& unboxedValue)
    {
        static_assert(
            ValidBoxV<TBoxed, TBoxedSize, TBoxedAlignment, TUnboxed>,
            "TUnobxed cannot be boxed into TBoxed");

        Destroy();

        m_cachedPointer = new (&m_storage) TUnboxed{std::move(unboxedValue)};
        m_mover = Mover<TUnboxed>;

        return *this;
    }

    // Main operator to access held interface
    //
    inline TBoxed* operator->() noexcept
    {
        return m_cachedPointer;
    }

private:
    // Implementation of a wrapper around move constructor for help object
    //
    template<typename TUnboxed>
    static TBoxed* Mover(void* source, void* target) noexcept
    {
        return new (target) TUnboxed{std::move(*reinterpret_cast<TUnboxed*>(source))};
    }

    // Helper method to destroy held object
    //
    inline void Destroy() noexcept
    {
        if (m_cachedPointer)
        {
            std::exchange(m_cachedPointer, nullptr)->~TBoxed();
        }
    }

    // To trick around MSVC being able to align right introducing left padding,
    // storage for the object goes first.
    //
    alignas(TBoxedAlignment) unsigned char m_storage[TBoxedSize] {'\0'};

    // Casted object pointer in case of multiple inheritance.
    //
    TBoxed* m_cachedPointer = nullptr;

    // Move constructor wrapper.
    //
    TBoxedMover* m_mover = nullptr;
};

// Helper type for defining a box size.
// Box users should define them as a full template specialization
// where TBoxed is a type of desired interface.
//
template <typename TBoxed>
struct BoxSize {
    static constexpr std::size_t Alignment = 0;
    static constexpr std::size_t Size = 0;
};

// Final box type for users
//
template <typename TBoxed>
using Box = BoxImpl<TBoxed, BoxSize<TBoxed>::Size, BoxSize<TBoxed>::Alignment>;

} // namespace dramcryx
