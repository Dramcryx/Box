#include "../Box.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

struct IBase {
	virtual ~IBase() = default;

	virtual int ImplSignature() = 0;
};

struct Base1 : IBase {
	Base1() noexcept = default;

	~Base1() override = default;

	Base1(Base1&&) noexcept = default;

	int ImplSignature() override
	{
		return *m_sig;
	}

private:
	std::unique_ptr<int> m_sig{std::make_unique<int>(1)};
};

struct Base2 : IBase {
	Base2() noexcept = default;

	~Base2() override = default;

	Base2(Base2&&) noexcept = default;

	int ImplSignature() override
	{
		return 2;
	}
};

struct VBase1 : virtual IBase {
	VBase1() noexcept = default;

	~VBase1() override = default;

	VBase1(VBase1&&) noexcept = default;

	int ImplSignature() override
	{
		return 3;
	}
};

struct VBase2 : virtual IBase {
	VBase2() noexcept = default;

	~VBase2() override = default;

	VBase2(VBase2&&) noexcept = default;

	int ImplSignature() override
	{
		return 4;
	}
};

struct VBase3 : VBase1, VBase2 {
	VBase3() noexcept = default;

	~VBase3() override = default;

	VBase3(VBase3&&) noexcept = default;

	int ImplSignature() override
	{
		return 5;
	}
};

template <>
struct dramcryx::BoxSize<IBase> {
	static constexpr std::size_t Size = 32;
	static constexpr std::size_t Alignment = 8;
};

template <>
struct dramcryx::BoxSize<std::string> {
	static constexpr std::size_t Size = 48;
	static constexpr std::size_t Alignment = 8;
};

int main()
{
	using namespace dramcryx;

	Box<IBase> box{Base1{}};
	assert(box->ImplSignature() == 1);

	box = Box<IBase>{Base2{}};
	assert(box->ImplSignature() == 2);

	box = Box<IBase>(VBase1{});
	assert(box->ImplSignature() == 3);

	box = Box<IBase>(VBase2{});
	assert(box->ImplSignature() == 4);

	box = Box<IBase>(VBase3{});
	assert(box->ImplSignature() == 5);

	box = Base1{};
	assert(box->ImplSignature() == 1);

	std::vector<Box<IBase>> boxes;
	boxes.emplace_back(Base1{});
	boxes.emplace_back(VBase1{});
	boxes.emplace_back(VBase3{});

	assert(boxes[0]->ImplSignature() == 1);
	assert(boxes[1]->ImplSignature() == 3);
	assert(boxes[2]->ImplSignature() == 5);

	return 0;
}
