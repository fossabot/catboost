#pragma once

#include "oblivious_model_progress.h"
#include <ostream>

template <class TInner>
class TAdditiveModel {
public:
    yvector<TInner> WeakModels;

    TAdditiveModel() {
    }

    virtual ~TAdditiveModel() {
    }

    template <class TDataSet, class TCursor>
    void Append(const TDataSet& ds,
                TCursor& cursor) const {
        for (size_t i = 0; i < WeakModels.size(); ++i) {
            WeakModels[i].Append(ds, cursor);
        }
    }

    void Rescale(const double scale) {
        for (auto& weakModel : WeakModels) {
            weakModel.Rescale(scale);
        }
    }

    void AddWeakModel(TInner&& weak) {
        WeakModels.push_back(std::move(weak));
    }

    void AddWeakModel(const TInner& weak) {
        WeakModels.push_back(weak);
    }

    const TInner& GetWeakModel(int i) const {
        return WeakModels[i];
    }

    const TInner& operator[](int i) const {
        return WeakModels[i];
    }

    size_t Size() const {
        return WeakModels.size();
    }

    double Value(const yvector<float>& point) const {
        double value = 0.0;
        for (uint i = 0; i < WeakModels.size(); i++)
            value += (double)WeakModels[i].Value(point);
        return value;
    }

    double Value(yvector<float>::const_iterator point) const {
        double value = 0.0;
        for (uint i = 0; i < WeakModels.size(); i++)
            value += (double)WeakModels[i].Value(point);
        return value;
    }
};

template <class TWeak>
inline std::ostream& operator<<(std::ostream& os, const TAdditiveModel<TWeak>& model) {
    for (uint i = 0; i < model.WeakModels.size(); i++) {
        if (i > 0)
            os << "+" << std::endl;
        os << model.WeakModels[i] << std::endl;
    }
    return os;
}
