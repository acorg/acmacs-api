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

    struct ChartAceProjection : public Object
    {
        String* get_c() const;
        Array*  get_l() const;
        size_t  get_i() const;
        double  get_s() const;
        String* get_m() const;
        Array*  get_C() const;
        Array*  get_t() const;
        Array*  get_g() const;
        Array*  get_f() const;
        bool    get_d() const;
        double  get_e() const;
        Array*  get_U() const;
        Array*  get_D() const;
        Array*  get_u() const;
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

    class Layout : public LayoutBase
    {
     public:
        inline Layout(const client::Array* aData) : mData(aData) {}

        inline LayoutBase* clone() const override { return new Layout{mData}; }
        inline const Coordinates& operator[](size_t aIndex) const override { const auto& coord = *(client::Array*)(*mData)[aIndex]; auto& result = * new Coordinates(coord.get_length()); for (size_t dim = 0; dim < coord.get_length(); ++dim) { result[dim] = coord[dim]->valueOf<double>(); } return result; }
        inline void set(size_t /*aIndex*/, const Coordinates& /*aCoordinates*/) override { log_error("Layout.set: Not implemented"); }
        inline size_t number_of_points() const override { return mData->get_length(); }
        inline size_t number_of_dimensions() const override { for (size_t p = 0; p < mData->get_length(); ++p) { const auto row_size = static_cast<client::Array*>((*mData)[p])->get_length(); if (row_size > 0) return row_size; } log_warning("Layout.number_of_dimensions: no points have coordinates"); return 0; }
        inline bool empty() const override { return mData->get_length() == 0; }

     private:
        const client::Array* mData;

    }; // class Layout

      // ----------------------------------------------------------------------

    class MinimumColumnBasis : public MinimumColumnBasisBase
    {
     public:
        inline MinimumColumnBasis(const String* aData) : mData(aData) {}

        inline operator size_t() const override { if (eq(mData, "none") || eq(mData, "auto")) return 0; return std::stoul(from_String(mData)); }
        inline operator std::string() const override { return from_String(mData); }

     private:
        const String* mData;

    }; // class MinimumColumnBasis

      // ----------------------------------------------------------------------

    class ColumnBases : public ColumnBasesBase
    {
     public:
        inline ColumnBases(const client::Array* aData) : mData(aData) {}

        inline void operator = (const ColumnBasesBase& aSrc) override { log_error("ColumnBases.=: Not implemented"); }
        inline double operator[](size_t aIndex) const override { return at(aIndex); }
        inline double at(size_t aIndex) const override { return *(*mData)[aIndex]; }
        inline void set(size_t aIndex, double aValue) override { log_error("ColumnBases.set: Not implemented"); }
        inline void clear() override { log_error("ColumnBases.clear: Not implemented"); }
        inline bool empty() const override { return mData->get_length() == 0; }
        inline size_t size() const override { return mData->get_length(); }
        inline void resize(size_t aNewSize) override { log_error("ColumnBases.resize: Not implemented"); }

     private:
        const client::Array* mData;

    }; // class ColumnBases

      // ----------------------------------------------------------------------

    class Projection : public ProjectionBase
    {
     public:
        inline Projection(const client::ChartAceProjection* aData) : mData(aData) {}

        inline std::string comment() const override { return from_String(mData->get_c()); }
        inline const LayoutBase& layout() const override { return * new Layout{mData->get_l()}; }
        inline double stress() const override { return mData->get_s(); }
        inline const MinimumColumnBasisBase& minimum_column_basis() const override { return * new MinimumColumnBasis{mData->get_m()}; }
        inline const ColumnBasesBase& column_bases() const override { return * new ColumnBases{mData->get_C()}; }
        inline bool dodgy_titer_is_regular() const override { return mData->get_d(); }
        inline const Transformation& transformation() const override { const auto& tr = *mData->get_t(); return * new Transformation{*tr[0], *tr[1], *tr[2], *tr[3]};  }
        inline double stress_diff_to_stop() const override { return mData->get_e(); }

     private:
        const client::ChartAceProjection* mData;

    }; //  class Projection

      // ----------------------------------------------------------------------

    inline const ChartInfoBase& chart_info() const override { return * new Info{mData->get_c()->get_i()}; }
    inline const AntigenBase& antigen(size_t ag_no) const override { return * new Antigen{(client::ChartAceAntigen*)mData->get_c()->get_a()->operator[](ag_no)}; }
    inline const SerumBase& serum(size_t sr_no) const override { return * new Serum{(client::ChartAceSerum*)mData->get_c()->get_a()->operator[](sr_no)}; }
    inline const ProjectionBase& projection(size_t aProjectionNo) const override { return * new Projection{(client::ChartAceProjection*)mData->get_c()->get_P()->operator[](aProjectionNo)}; }

    inline String* name() { return static_cast<const Info&>(chart_info()).make_name_S(); }

 private:
    client::ChartAceData* mData;

}; // class ChartAce

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
