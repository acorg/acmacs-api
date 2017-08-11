#pragma once

#include <cheerp/clientlib.h>

#include "acmacs-base/passage.hh"
#include "acmacs-chart/chart-base.hh"
#include "string.hh"

// ----------------------------------------------------------------------

class LocDb;

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

    struct ChartAceAntigen : public Object
    {
        String* get_N() const;
        String* get_D() const;
        String* get_L() const;
        String* get_P() const;
        String* get_R() const;
        Array*  get_l() const;
        String* get_S() const;
        Array*  get_a() const;
        Array*  get_c() const;
    };

    struct ChartAceSerum : public Object
    {
        String* get_N() const;
        String* get_L() const;
        String* get_P() const;
        String* get_R() const;
        String* get_I() const;
        String* get_S() const;
        Array*  get_h() const;
        Array*  get_a() const;
        String* get_s() const;
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
                    return concat_space(mData->get_l(), mData->get_V(), lineage_S(), mData->get_A(), mData->get_r(), mData->get_D());
                else
                    return mData->get_N();
            }

        inline const std::string make_name() const override { return (std::string)*make_name_S(); }

     private:
        const client::ChartAceInfo* mData;

    }; // class Info

      // ----------------------------------------------------------------------

    class Antigen : public AntigenBase
    {
     public:
        inline Antigen(const client::ChartAceAntigen* aData) : mData(aData) {}

        inline bool reference() const override { return client::is_defined(mData->get_S()) && mData->get_S()->indexOf("R"_S) >= 0; }
        inline std::string full_name() const override { return from_String(concat_space(mData->get_N(), mData->get_R(), join_array(mData->get_a()), mData->get_P())); }
        inline std::string full_name_without_passage() const override { return from_String(concat_space(mData->get_N(), mData->get_R(), join_array(mData->get_a()))); }
        inline std::string abbreviated_name(const LocDb&) const override { log_error("Antigen.abbreviated_name: Not implemented"); return std::string{}; }
        inline const std::string name() const override { return from_String(mData->get_N()); }
        inline const std::string lineage() const override { return from_String(mData->get_L()); }
        inline const std::string passage() const override { return from_String(mData->get_P()); }
        inline bool has_passage() const override { return client::is_not_empty(mData->get_P()); }
        inline std::string passage_without_date() const override { log_error("Antigen.passage_without_date: Not implemented"); return std::string{}; }
        inline bool is_egg() const override { return passage::is_egg(passage()) || is_reassortant(); }
        inline const std::string reassortant() const override { return from_String(mData->get_R()); }
        inline bool is_reassortant() const override { return client::is_not_empty(mData->get_R()); }
        inline bool distinct() const override { return client::is_defined(mData->get_a()) && mData->get_a()->indexOf("DISTINCT"_S) >= 0; }

        inline AntigenSerumMatch match(const AntigenSerumBase& aNother) const override { log_error("Antigen.match: Not implemented"); return AntigenSerumMatch{AntigenSerumMatch::NameMismatch}; }
        inline AntigenSerumMatch match_passage(const AntigenSerumBase& aNother) const override { log_error("Antigen.match_passage: Not implemented"); return AntigenSerumMatch{AntigenSerumMatch::NameMismatch}; }

     private:
        const client::ChartAceAntigen* mData;

    }; // class Antigen

      // ----------------------------------------------------------------------

    class Serum : public SerumBase
    {
     public:
        inline Serum(const client::ChartAceSerum* aData) : mData(aData) {}

        inline const std::vector<size_t>& homologous() const override { log_error("Serum.homologous: Not implemented"); return * new std::vector<size_t>{}; }
        inline std::string full_name() const override { return from_String(concat_space(mData->get_N(), mData->get_R(), mData->get_I(), join_array(mData->get_a()))); }
        inline std::string full_name_without_passage() const override { return full_name(); }
        inline std::string abbreviated_name(const LocDb&) const override { log_error("Serum.abbreviated_name: Not implemented"); return std::string{}; }
        inline const std::string name() const override { return from_String(mData->get_N()); }
        inline const std::string lineage() const override { return from_String(mData->get_L()); }
        inline const std::string passage() const override { return from_String(mData->get_P()); }
        inline bool has_passage() const override { return client::is_not_empty(mData->get_P()); }
        inline bool is_egg() const override { return passage::is_egg(passage()) || is_reassortant(); }
        inline std::string passage_without_date() const override { log_error("Serum.passage_without_date: Not implemented"); return std::string{}; }
        inline const std::string reassortant() const override { return from_String(mData->get_R()); }
        inline bool is_reassortant() const override { return client::is_not_empty(mData->get_R()); }
        inline bool distinct() const override { return client::is_defined(mData->get_a()) && mData->get_a()->indexOf("DISTINCT"_S) >= 0; }
        inline AntigenSerumMatch match(const AntigenSerumBase& aNother) const override { log_error("Serum.match: Not implemented"); return AntigenSerumMatch{AntigenSerumMatch::NameMismatch}; }
        inline AntigenSerumMatch match_passage(const AntigenSerumBase& aNother) const override { log_error("Serum.match_passage: Not implemented"); return AntigenSerumMatch{AntigenSerumMatch::NameMismatch}; }

     private:
        const client::ChartAceSerum* mData;

    }; // class Serum

      // ----------------------------------------------------------------------

    inline const ChartInfoBase& chart_info() const override { return * new Info{mData->get_c()->get_i()}; }
    inline const AntigenBase& antigen(size_t ag_no) const override { return * new Antigen{(client::ChartAceAntigen*)mData->get_c()->get_a()->operator[](ag_no)}; }
    inline const SerumBase& serum(size_t sr_no) const override { return * new Serum{(client::ChartAceSerum*)mData->get_c()->get_a()->operator[](sr_no)}; }
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
