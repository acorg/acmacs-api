#pragma once

#include "command.hh"

class AcmacsC2;

// ----------------------------------------------------------------------

class Command_doc : public Command
{
 public:
    using Command::Command;

    void run() override;

    std::string get_id_str() const { return data()["id"]; }
    auto get_id() const { return bsoncxx::oid{get_id_str()}; }
    auto get_chain_source_data() const { return rjson::get_or(data(), "chain_source_data", false); }

    static const char* description();

 private:
    void chain_source_data(bsoncxx::document::value& doc);
    void append_source_data(bsoncxx::oid&& id, bsoncxx::builder::basic::array& container);

}; // class Command_chart

// ----------------------------------------------------------------------

class Command_root_charts : public Command
{
 public:
    using Command::Command;

    void run() override;

    int get_chunk_size() const { return rjson::get_or(data(), "chunk_size", 0); }
    int get_skip() const { return rjson::get_or(data(), "skip", 0); }
    int get_limit() const { return rjson::get_or(data(), "limit", 0); }
    const rjson::value& get_owners() const { return data()["owners"]; }
    const rjson::value& get_keywords() const { return data()["keywords"]; }
    const rjson::value& get_search() const { return data()["search"]; }

    static const char* description();

}; // class Command_root_charts

// ----------------------------------------------------------------------

class Command_chart_keywords : public Command
{
 public:
    using Command::Command;
    void run() override;
    static const char* description();

}; // class Command_chart_keywords

// ----------------------------------------------------------------------

class Command_chart_owners : public Command
{
 public:
    using Command::Command;
    void run() override;
    static const char* description();

}; // class Command_chart_owners

// ----------------------------------------------------------------------

class Command_with_c2_access : public Command
{
 public:
    Command_with_c2_access(rjson::value&& aSrc, MongoAcmacsC2Access& aMongoAccess, ClientConnection& aClientConnection, size_t aCommandNumber);

    std::string get_id_str() const { return data()["id"]; }
    auto get_id() const { return bsoncxx::oid{get_id_str()}; }

 protected:
    auto& c2() { return acmacs_c2_; }

 private:
    AcmacsC2& acmacs_c2_;

}; // class Command_with_c2_access

// ----------------------------------------------------------------------

class Command_chart : public Command_with_c2_access
{
 public:
    using Command_with_c2_access::Command_with_c2_access;
    void run() override;
    static const char* description();

}; // class Command_chart

// ----------------------------------------------------------------------

class Command_ace : public Command_with_c2_access
{
 public:
    using Command_with_c2_access::Command_with_c2_access;
    void run() override;
    static const char* description();

}; // class Command_ace

// ----------------------------------------------------------------------

class Command_pdf : public Command_with_c2_access
{
 public:
    using Command_with_c2_access::Command_with_c2_access;
    void run() override;
    static const char* description();

}; // class Command_pdf

// ----------------------------------------------------------------------

class Command_download_ace : public Command_with_c2_access
{
 public:
    using Command_with_c2_access::Command_with_c2_access;

    void run() override;
    static const char* description();

}; // class Command_download_ace

// ----------------------------------------------------------------------

class Command_download_lispmds_save : public Command_with_c2_access
{
 public:
    using Command_with_c2_access::Command_with_c2_access;
    void run() override;
    static const char* description();

}; // class Command_download_lispmds_save

// ----------------------------------------------------------------------

class Command_download_layout : public Command_with_c2_access
{
 public:
    using Command_with_c2_access::Command_with_c2_access;
    void run() override;
    static const char* description();
};

// ----------------------------------------------------------------------

class Command_download_table_map_distances : public Command_with_c2_access
{
 public:
    using Command_with_c2_access::Command_with_c2_access;
    void run() override;
    static const char* description();
};

// ----------------------------------------------------------------------

class Command_download_error_lines : public Command_with_c2_access
{
 public:
    using Command_with_c2_access::Command_with_c2_access;
    void run() override;
    static const char* description();
};

// ----------------------------------------------------------------------

class Command_download_distances_between_all_points : public Command_with_c2_access
{
 public:
    using Command_with_c2_access::Command_with_c2_access;
    void run() override;
    static const char* description();
};

// ----------------------------------------------------------------------

class Command_sequences_of_chart : public Command_with_c2_access
{
 public:
    using Command_with_c2_access::Command_with_c2_access;
    void run() override;
    static const char* description();
};

// ----------------------------------------------------------------------

class Command_download_sequences_of_chart_as_fasta : public Command_with_c2_access
{
 public:
    using Command_with_c2_access::Command_with_c2_access;
    void run() override;
    static const char* description();
};

// ----------------------------------------------------------------------

class Command_download_layout_sequences_as_csv : public Command_with_c2_access
{
 public:
    using Command_with_c2_access::Command_with_c2_access;
    void run() override;
    static const char* description();
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
