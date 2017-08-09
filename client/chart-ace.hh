#pragma once

#include <cheerp/clientlib.h>

#include "acmacs-chart/chart-base.hh"
#include "string.hh"

// ----------------------------------------------------------------------

namespace client
{
    struct ChartAceInfo : public Object
    {
        String* get_v() const;
        String* get_V() const;
        String* get_A() const;
        String* get_D() const;
        String* get_N() const;
        String* get_l() const;
        String* get_r() const;
        String* get_s() const;
        String* get_T() const;
        Array*  get_S() const;
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

    inline size_t number_of_antigens() const override { return mData->number_of_antigens(); }
    inline size_t number_of_sera() const override { return mData->number_of_sera(); }
    inline size_t number_of_projections() const override { return mData->number_of_projections(); }

    class Info : public ChartInfoBase
    {
     public:
        Info(const client::ChartAceInfo* aData) : mData(aData) {}

        inline const std::string virus() const override { return from_String(mData->get_v()); }
        inline const std::string virus_type() const override { return from_String(mData->get_V()); }
        inline const std::string assay() const override { return from_String(mData->get_A()); }
        inline const std::string lab() const override { return from_String(mData->get_l()); }
        inline const std::string rbc() const override { return from_String(mData->get_r()); }
        inline const std::string name() const override { return from_String(mData->get_N()); }
        inline const std::string subset() const override { return from_String(mData->get_s()); }
        inline const std::string date() const override { return from_String(mData->get_D()); }

        inline String* lineage_S() const { return new String(); }

        inline String* make_name_S() const
            {
                if (is_undefined_or_null(mData->get_N()))
                    return concat(mData->get_l(), " ", mData->get_V(), " ", lineage_S(), " ", mData->get_A(), " ", mData->get_r(), " ", mData->get_D());
                else
                    return mData->get_N();
            }

        inline const std::string make_name() const override { return (std::string)*make_name_S(); }

     private:
        const client::ChartAceInfo* mData;

    }; // class Info

    inline const ChartInfoBase& chart_info() const override { return * new Info{mData->get_c()->get_i()}; }
    inline const AntigenBase& antigen(size_t ag_no) const  override {}
    inline const SerumBase& serum(size_t sr_no) const override {}
    inline ProjectionBase& projection(size_t aProjectionNo) override {}
    inline const ProjectionBase& projection(size_t aProjectionNo) const override {}

    inline String* name() { return static_cast<const Info&>(chart_info()).make_name_S(); }

 private:
    client::ChartAceData* mData;

}; // class ChartAce

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
