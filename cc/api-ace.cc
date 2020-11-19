#include "acmacs-base/argv.hh"
#include "acmacs-base/read-file.hh"
#include "hidb-5/vaccines.hh"
#include "seqdb-3/seqdb.hh"
#include "acmacs-chart-2/ace-export.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-chart-2/chart-modify.hh"

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    argument<str> input_chart{*this, arg_name{"input-chart-file"}, mandatory};
    argument<str> output_ace{*this, arg_name{"output-uncompressed-instrumented-ace"}};
};

int main(int argc, char* const argv[])
{
    using namespace std::string_view_literals;
    int exit_code = 0;
    try {
        Options opt(argc, argv);
        acmacs::chart::ChartModify chart{acmacs::chart::import_from_file(opt.input_chart)};
        chart.antigens_modify().set_continent();
        hidb::update_vaccines(chart); // updates semantic attirubutes (not implemented)
        acmacs::seqdb::get().add_clades(chart, acmacs::verbose::yes);
        const auto exported = export_ace(chart, "mod_acmacs", 0);
        acmacs::file::write(opt.output_ace ? *opt.output_ace : "-", exported);
    }
    catch (std::exception& err) {
        fmt::print(stderr, "ERROR: {}\n", err);
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
