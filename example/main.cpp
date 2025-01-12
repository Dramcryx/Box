#include "declarations.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

struct Benchmark {
    long long ConstructionTime = 0;
    long long AccessTime = 0;
    long long DestructionTime = 0;

    void Compare(const Benchmark& other)
    {
        std::cout << "Constructed in " << ConstructionTime << " vs " << other.ConstructionTime << " ms, " << (1.f - (float)ConstructionTime / (float)other.ConstructionTime) * 100.f << "% diff\n"
                << "Accessed in " << AccessTime << " vs " << other.AccessTime << " ms, " << (1.f - (float)AccessTime / (float)other.AccessTime) * 100.f << "% diff\n"
                << "Destructed in " << DestructionTime << " vs " << other.DestructionTime << " ms, " << (1.f - (float)DestructionTime / (float)other.DestructionTime) * 100.f << "% diff\n";
    }
};

int main()
{
    using namespace std::chrono_literals;

    srand(42);

    static constexpr std::size_t CountCreated = 1000000;
    static constexpr std::size_t CountAccessed = 25000;

#define NOW std::chrono::high_resolution_clock::now()
#define DURATION(x) std::chrono::duration_cast<std::chrono::milliseconds>(x##End - x##Begin).count()

    auto ptrs = []{
        std::vector<std::unique_ptr<IBoxable>> boxes;
        boxes.reserve(CountCreated);

        auto constructedBegin = NOW;

        for (int i = 0; i < CountCreated; ++i)
        {
            boxes.push_back(std::make_unique<VirtualBoxed>());
        }

        auto constructedEnd = NOW;

        auto accessBegin = NOW;
        for (int i = 0; i < CountAccessed; ++i)
        {
            boxes[rand() % CountCreated]->WriteBoxed();
        }

        auto accessEnd = NOW;

        auto destructionBegin = NOW;
        boxes.clear();
        auto destructionEnd = NOW;
        
        return Benchmark{
            DURATION(constructed),
            DURATION(access),
            DURATION(destruction)};
    }();

    auto boxes = []{
        std::vector<dramcryx::Box<IBoxable>> boxes;
        boxes.reserve(CountCreated);

        auto constructedBegin = NOW;

        for (int i = 0; i < CountCreated; ++i)
        {
            boxes.push_back(BoxCreator());
        }

        auto constructedEnd = NOW;

        auto accessBegin = NOW;
        for (int i = 0; i < CountAccessed; ++i)
        {
            boxes[rand() % CountCreated]->WriteBoxed();
        }

        auto accessEnd = NOW;

        auto destructionBegin = NOW;
        boxes.clear();
        auto destructionEnd = NOW;
        
        return Benchmark{
            DURATION(constructed),
            DURATION(access),
            DURATION(destruction)};
    }();

    std::cout << "Unique ptr vs Boxed:\n";
    ptrs.Compare(boxes);

    return 0;
}
