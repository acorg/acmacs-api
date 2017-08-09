#pragma once

#include <cheerp/clientlib.h>

#include "acmacs-chart/chart-base.hh"

// ----------------------------------------------------------------------

namespace client
{
    struct ChartAceInfo : public Object
    {
        String* get_N() const;
    };

    struct ChartAceChart : public Object
    {
        const ChartAceInfo* get_i() const;
        const Array* get_a() const;
        const Array* get_s() const;
        const Array* get_P() const;
    };

    struct ChartAceData : public Object
    {
        const ChartAceChart* get_c() const;

        inline String* name() const { return get_c()->get_i()->get_N(); }
        inline size_t number_of_antigens() const { return get_c()->get_a()->get_length(); }
        inline size_t number_of_sera() const { return get_c()->get_s()->get_length(); }
        inline size_t number_of_projections() const { return get_c()->get_P()->get_length(); }
    };
}

// ----------------------------------------------------------------------

class ChartAce : public ChartBase
{
 public:
    inline ChartAce(client::ChartAceData* aData) : mData(aData) {}

    inline String* name() { return mData->name(); }

    inline size_t number_of_antigens() const override { return mData->number_of_antigens(); }
    inline size_t number_of_sera() const override { return mData->number_of_sera(); }
    inline size_t number_of_projections() const override { return mData->number_of_projections(); }

    inline const ChartInfoBase& chart_info() const override {}
    inline const AntigenBase& antigen(size_t ag_no) const  override {}
    inline const SerumBase& serum(size_t sr_no) const override {}
    inline ProjectionBase& projection(size_t aProjectionNo) override {}
    inline const ProjectionBase& projection(size_t aProjectionNo) const override {}

 private:
    client::ChartAceData* mData;

}; // class ChartAce

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
