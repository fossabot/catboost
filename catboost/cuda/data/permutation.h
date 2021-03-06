#pragma once

#include <catboost/cuda/cuda_util/cpu_random.h>
#include <util/system/types.h>
#include <util/generic/vector.h>
#include <catboost/cuda/cuda_lib/slice.h>
#include <numeric>
#include "data_provider.h"

class TDataPermutation {
private:
    ui32 Index;
    ui32 Size;
    ui32 BlockSize;
protected:
    TDataPermutation(ui32 index,
                     ui32 poolSize,
                     ui32 blockSize
    )
        : Index(index)
        , Size(poolSize)
        , BlockSize(blockSize) {

    }

    friend TDataPermutation GetPermutation(const TDataProvider& dataProvider, ui32 permutationId, ui32 blockSize);

public:
    void FillOrder(yvector<ui32>& order) const {
        order.resize(Size);
        std::iota(order.begin(), order.end(), 0);

        if (Index != IdentityPermutationId()) {
            TRandom rng(1664525 * GetPermutationId() + 1013904223);
            if (BlockSize == 1)
            {
                std::random_shuffle(order.begin(), order.begin() + Size, rng);
            } else {
                const ui32 blocksCount = static_cast<ui32>(NHelpers::CeilDivide(order.size(), BlockSize));
                yvector<ui32> blocks(blocksCount);
                std::iota(blocks.begin(), blocks.end(), 0);
                std::random_shuffle(blocks.begin(), blocks.end(), rng);

                ui32 cursor = 0;
                for (ui32 i = 0; i < blocksCount; ++i) {
                    const ui32 blockStart = blocks[i] * BlockSize;
                    const ui32 blockEnd = Min<ui32>(blockStart + BlockSize, order.size());
                    for (ui32 j = blockStart; j < blockEnd; ++j) {
                        order[cursor++] = j;
                    }
                }
            }
        }
    }

    void FillInversePermutation(yvector<ui32>& permutation) const {
        yvector<ui32> order;
        FillOrder(order);
        permutation.resize(order.size());
        for (ui32 i = 0; i < order.size(); ++i) {
            permutation[order[i]] = i;
        }
    }

    template <class TBuffer>
    void WriteOrder(TBuffer& dst) const {
        yvector<ui32> order;
        FillOrder(order);
        dst.Write(order);
    }

    template <class TBuffer>
    void WriteInversePermutation(TBuffer& dst) const {
        yvector<ui32> order;
        FillInversePermutation(order);
        dst.Write(order);
    }

    bool IsIdentity() {
        return Index == IdentityPermutationId();
    }

    static constexpr ui32 IdentityPermutationId() {
        return 0;
    }

    ui32 GetPermutationId() const {
        return Index;
    }

    TSlice ObservationsSlice() const {
        return TSlice(0, Size);
    }
};

inline TDataPermutation GetPermutation(const TDataProvider& dataProvider,
                                       ui32 permutationId,
                                       ui32 blockSize = 1
) {
    return TDataPermutation(permutationId,
                            dataProvider.GetSampleCount(),
                            blockSize
    );
}
