import * as acv_utils from "../map-draw/ace-view-1/utils.js";
import * as acv_toolkit from "../map-draw/ace-view-1/toolkit.js";
import * as api_utils from "./utils.js";

// ----------------------------------------------------------------------

export function chains(data, dispatcher) {
    acmacs_web_title("Acmacs-Web Chains", true);
    let chains = new Chains($("<div class='chains'></div>").appendTo($("body")), data, dispatcher);
    $("head title").empty().append("AW: chains");
}

// ----------------------------------------------------------------------

export function chain(data, dispatcher) {
    acmacs_web_title(data.description || data._id, true);
    let chains = new Chain($("<div class='chain'></div>").appendTo($("body")), data, dispatcher);
    $("head title").empty().append("AW: chain " + (data.description || data._id));
}

// ----------------------------------------------------------------------

export function chain_step(data, step_no, dispatcher) {
    acmacs_web_title(data.description || data._id, true);
    let chains = new ChainStep($("<div class='chain-step'></div>").appendTo($("body")), data, step_no, dispatcher);
    $("head title").empty().append(`AW: chain ${(data.description || data._id)} step ${step_no + 1}`);
}

// ----------------------------------------------------------------------

const sChainType = {
    "acmacs.inspectors.routine_diagnostics.IncrementalChainForked": "",
    "acmacs.inspectors.routine_diagnostics.IncrementalChain": "",
    "acmacs.inspectors.optimization.MapResolutionDeterminationByComputationalTiterPrediction": "MapRes",
    "acmacs.inspectors.routine_diagnostics.RoutineDiagnosticsChain": "RD",
    "acmacs.inspectors.optimization.OptimalMinimumColumnBasis": "OptMinColBasis",
    "acmacs.inspectors.optimization.DimensionTest": "Dim",
    "acmacs.inspectors.avidity.AvidityChain": "Avidity",
    "acmacs.inspectors.hemi_local.HemiGridChain": "Grid",
    "acmacs.inspectors.optimization.OptimizationExisting": "OptExisting"
};

const sVirusTypes = {"H1pdm": "H1pdm", "A(H1N1)2009pdm": "H1pdm", "H1": "H1", "A(H3N2)": "H3", "H3": "H3", "B/Vic": "B/Vic", "B/Yam": "B/Yam"};
const sVirusTypeOrder = ["H1pdm", "H1", "H3 HI", "H3 Neut", "B/Vic", "B/Yam", "Other"];
const sLabs = {CDC: "CDC", MELB: "VIDRL", NIID: "NIID", NIMR: "Crick", CNIC: "CNIC"};
const sLabsOrder = ["CDC", "Crick", "NIID", "VIDRL", "CNIC", "Other"];

class Chains {

    constructor(node, data, dispatcher) {
        this.node = node;
        this.chains_total = data.chain_count;
        this.dispatcher = dispatcher;
        this.split_by_virus_type_lab_(data);
        this.find_forked_();
        this.show();
    }

    show() {
        console.log("Total chains: " + this.chains_total);
        // this.node.append(`<div class='chains-title'>Chains: ${this.chains_total}</div>`);
        let vt_ul = $("<ul class='chains-virus-types'></ul>").appendTo(this.node);
        for (let virus_type of sVirusTypeOrder)
            this.show_vt_(vt_ul, virus_type, this.data[virus_type]);
    }

    show_vt_(node, virus_type, entries) {
        if (entries !== undefined) {
            let vt_li = $(`<li class='adt-shadow'><div class='chains-virus-type-name'>${virus_type}</div></li>`).appendTo(node);
            let lab_ul = $("<ul class='chains-labs'></ul>").appendTo(vt_li);
            for (let lab of sLabsOrder)
                this.show_lab_(lab_ul, lab, entries[lab]);
        }
    }

    show_lab_(node, lab, entries) {
        if (entries !== undefined) {
            let lab_li = $(`<li><div class='chains-lab-name'>${lab}</div></li>`).appendTo(node);
            let chain_list = $("<ol class='chains-chains'></ol>").appendTo(lab_li);
            for (let chain of entries)
                this.show_chain_($("<li></li>").appendTo(chain_list), chain);
        }
    }

    show_chain_(node, chain) {
        const span_name = `<a href="${api_utils.url_prefix()}chain/${chain._id}" target="_blank" class='chains-chain-name'>${chain.name}</a>`;
        const span_id = `<span class='chains-chain-id ads-id-popup'>${chain._id}</span>`;
        let classes = this.keyword_classes_(chain);
        const title = (chain.keywords && chain.keywords.length) ? "keywords: " + JSON.stringify(chain.keywords) : "";

        const chain_row = $(`<span class='${classes}' title='${title}'>${this.span_state_(chain)}${span_name}${this.span_modification_time_(chain)}${span_id}</span>`).appendTo(node);
        chain_row.find(".chains-chain-id").on("click", evt => acv_toolkit.movable_window_with_json(chain, evt.target, chain.name || chain.description || chain._id));
        if (chain.forked_parent) {
            const sp = $(`<div class='chains-chain-fork-of'><span class='chains-chain-fork-of-prefix'>fork at <span class='chains-chain-fork-at'>${chain.forked_step}</span> of </span></div>`).appendTo(chain_row);
            this.show_chain_(sp, chain.forked_parent);
        }
    }

    span_state_(chain) {
        switch (chain.state) {
        case "FAILED":
            return "<span class='chain-state chain-state-FAILED' title='FAILED'>F</span>";
        case "handling":
            return "<span class='chain-state chain-state-handling' title='handling'>H</span>";
        case "waiting":
            return "<span class='chain-state chain-state-waiting' title='waiting'>W</span>";
        case "completed":
            return ""; // `<span class='chain-state chain-state-${chain.state}' title='completed'> </span>`;
        default:
            return "";
        }
    }

    span_modification_time_(chain) {
        if (chain._m) {
            let m_classes = ["chains-chain-m"];
            const oldness = new Date() - new Date(chain._m.substr(0, 10));
            if (oldness > (1000 * 60 * 60 * 24 * 183)) // 1/2 year
                m_classes.push("chains-chain-m-older-six-months");
            else if (oldness > (1000 * 60 * 60 * 24 * 30)) // 30 days
                m_classes.push("chains-chain-m-older-one-month");
            return `<span class='${m_classes.join(" ")}' title='${chain._m.substr(0, chain._m.indexOf("."))}'>[${chain._m.substr(0, 10)}]</span>`;
        }
        else
            return "";
    }

    keyword_classes_(chain) {
        let classes = [];
        for (let kw of (chain.keywords || [])) {
            switch (kw) {
            case "whocc":
            case "degraded":
            case "no-min-col-basis":
            case "backward":
            case "neutralization":
            case "focus-reduction":
            case "microneutralization":
            case "sparse":
                classes.push("chains-chain-keyword-" + kw);
                break;
            default:
                console.warn("Unsupported chain keyword " + JSON.stringify(kw) + " of chain " + chain._id);
                break;
            }
        }
        return classes.join(" ");
    }

    split_by_virus_type_lab_(data) {
        this.data = {};         // virus_type -> lab -> [fields]
        for (let chain_entry of data.chains) {
            const fields = this.find_fields_(chain_entry);
            if (this.data[fields.virus_type] === undefined)
                this.data[fields.virus_type] = {};
            if (this.data[fields.virus_type][fields.lab] === undefined)
                this.data[fields.virus_type][fields.lab] = [];
            this.data[fields.virus_type][fields.lab].push(fields);
        }
    }

    find_fields_(entry) {
        if (entry.description) {
            entry.name = entry.description;
            entry.lab = this.find_replace_field_(sLabs, entry);
            entry.virus_type = this.find_replace_field_(sVirusTypes, entry);
            if (entry.virus_type === "H3") {
                const is_neut = (entry.keywords || []).filter(kw => ["neutralization", "microneutralization", "focus-reduction"].indexOf(kw) >= 0).length > 0;
                if (is_neut)
                    entry.virus_type += " Neut";
                else
                    entry.virus_type += " HI";
            }
        }
        else {
            entry.virus_type = "Other";
            entry.lab = "Other";
            entry.name = entry._id;
        }
        return entry;
    }

    find_replace_field_(collection, entry) {
        // const raw = Object.entries(collection).filter(kv => entry.description.indexOf(kv[0]) >= 0);
        let result, raw;
        for (let src in collection) {
            if (entry.description.indexOf(src) >= 0 && (raw === undefined || raw.length < src.length)) {
                raw = src;
                result = collection[src];
            }
        }
        if (raw !== undefined)
            entry.description = entry.description.replace(raw, result);
        else
            result = "Other";
        return result;
    }

    find_forked_() {
        for (let for_vt of Object.values(this.data)) {
            for (let for_lab of Object.values(for_vt)) {
                let to_remove = [];
                for (let entry of for_lab) {
                    if (entry.forked) {
                        const forked_parent = for_lab.filter(ee => ee._id === entry.forked);
                        if (forked_parent.length === 1) {
                            entry.forked_parent = forked_parent[0];
                            to_remove.push(entry.forked);
                        }
                    }
                }
                for (let to_rem of to_remove) {
                    const index = for_lab.findIndex(elt => elt._id === to_rem);
                    for_lab.splice(index, 1);
                }
            }
        }
    }
}

// ----------------------------------------------------------------------

const Chain_default_options = {steps_with_initial_maps: 2};
const Chain_cell_type_to_name = {i: "incremental", s: "from scratch", "1": "individual", "1m": "merge col bases"};

const Chain_acmacs_web_title_html = "\
<span class='chain-min-col-basis'>&gt;=${min_col_basis}</span>\
<span class='chain-date'><span class='chain-begin-date'></span><span class='chain-date-dash'>-</span><span class='chain-end-date'></span></span>\
<span class='chain-id ads-id-popup'>${id}</span>\
";

const Chain_html = "<ul class='chain'></ul>";

const Chain_source_row_html = "\
<li class='chain-row'>\
 <table><tr>\
 <td class='chain-row-step-no'>\
  <div class='chain-row-step-no'>\
   <div class='a-top'>\
    <div class='a-number'>${src_no}</div>\
    <div class='a-icon a-expand-icon' title='show maps'>&#x21CA;</div>\
   </div>\
   <div class='a-middle'>\
   </div>\
   <div class='a-bottom'>\
    <div class='a-icon a-collapse-icon' title='show maps'>&#x21C8;</div>\
   </div>\
  </div>\
 </td>\
 <td class='chain-row-content'>\
  <div>\
   <div class='chain-title-row'>\
    <a class='chain-step-title' target='_blank'>Step</a>\
    <span class='chain-step-id ads-id-popup'>${id}</span>\
   </div>\
   <table class='chain-step-maps'>\
    <tr class='chain-step-maps'></tr>\
   </table>\
  </div>\
 </td>\
 </tr></table>\
</li>\
";

class Chain {

    constructor(node, data, dispatcher, options={}) {
        // console.log("Chain.data", data);
        this.node = node;
        this.dispatcher = dispatcher;
        this.options = Object.assign({}, Chain_default_options, options);
        acmacs_web_title(acv_utils.format(Chain_acmacs_web_title_html, {id: data._id, min_col_basis: data.minimum_column_basis || "none"}), false);
        $("body .acmacs-web-header .acmacs-web-title .chain-id").on("click", evt => acv_toolkit.movable_window_with_json(data, evt.target, data.name || data.description || data._id));
        this.show(data);
    }

    show(data) {
        const table = $(Chain_html).appendTo(this.node);
        for (let src_no = data.sources.length - 1; src_no >= 0; --src_no) {
            const row = $(acv_utils.format(Chain_source_row_html, {src_no: src_no + 1, id: data.sources[src_no]})).appendTo(table);
            if (data.sources[src_no])
                this.make_source(row, src_no, data);
            if (data.results[src_no])
                this.make_results(row, src_no, data);
        }
    }

    make_results(row, step_no, data) {
        row.find(".a-expand-icon").on("click", () => this.expand_results(row, step_no, data));
        row.find(".a-collapse-icon").on("click", () => this.collapse_results(row, step_no, data));
        if (step_no >= (data.sources.length - this.options.steps_with_initial_maps))
            this.expand_results(row, step_no, data);
        else
            this.collapse_results(row, step_no, data);
    }

    expand_results(row, step_no, data) {
        for (let cell_type of ["i", "s", "1", "1m"]) {
            if (data.results[step_no][cell_type]) {
                const cell = this.make_map_cell(row.find("table.chain-step-maps tr"), cell_type);
                this.dispatcher.send_receive({C: "doc", id: data.results[step_no][cell_type]}, message => this.cell_map_add_content(cell, cell_type, message));
            }
        }
        row.addClass("chain-row-expanded").removeClass("chain-row-collapsed");
        row.find("table.chain-row").show();
    }

    collapse_results(row, step_no, data) {
        row.removeClass("chain-row-expanded").addClass("chain-row-collapsed");
        row.find("table.chain-row").hide();
        row.find("table.chain-step-maps tr").empty();
    }

    make_map_cell(node, cell_type) {
        return $(`<td><div class='map-cell-title'>${Chain_cell_type_to_name[cell_type]}<span class='chart-id ads-id-popup'></span></div></td>`).appendTo(node);
    }

    make_text_cell(node, cell_type) {
        let td = node.find("td");
        if (td.length === 0)
            td = $("<td></td>").appendTo(node);
        return $(`<div>${Chain_cell_type_to_name[cell_type]}: </div>`).appendTo(td);
    }

    cell_map_add_content(cell, cell_type, message) {
        cell.find("span.chart-id").empty().append(message.doc._id).on("click", evt => acv_toolkit.movable_window_with_json(message.doc, evt.target, message.doc.name || message.doc.description || message.doc._id));
        if (message.doc.stresses && message.doc.stresses.length) {
            // cell.find("div").append(" " + message.doc.stresses[0].toFixed(2));
            antigenic_map_widget(cell, message.doc._id, this.dispatcher);
        }
    }

    cell_text_add_content(cell, cell_type, message) {
        if (message.doc.stresses && message.doc.stresses.length)
            cell.append(message.doc.stresses[0].toFixed(2));
    }

    make_source(node, step_no, data) {
        const source_data = data.ad_api_source_data && data.ad_api_source_data[step_no];
        const name = (source_data && source_data.name) || "Table";
        node.find(".chain-step-title").empty().append(name).attr("href", api_utils.url_prefix() + "chain/" + data._id + "/" + step_no);
        node.find(".chain-step-id").on("click", evt => movable_window_with_json_for_id(data.sources[step_no], evt.target, this.dispatcher));
        if (step_no === 0 && source_data && source_data.date)
            $("body .acmacs-web-header .acmacs-web-title .chain-begin-date").empty().append(source_data.date);
        else if (step_no === (data.sources.length - 1) && source_data && source_data.date) {
            $("body .acmacs-web-header .acmacs-web-title .chain-end-date").empty().append(source_data.date);
            $("body .acmacs-web-header .acmacs-web-title .chain-date-dash").show();
        }
    }
}

// ----------------------------------------------------------------------

const ChainStep_source_html = "\
<tr>\
 <td>\
  <div>\
   <div class='chain-title-row'>\
    Chain source <span class='chain-step-id ads-id-popup'>${id}</span>\
   </div>\
  </div>\
 </td>\
</tr>\
";

const ChainStep_results_html = "\
<tr class='merge-incremental'><td><table class='adt-shadow'><tr></tr></table></td></tr>\
<tr class='merge-scratch'><td><table class='adt-shadow'><tr></tr></table></td></tr>\
<tr class='individual'><td><table class='adt-shadow'><tr></tr></table></td></tr>\
<tr class='individual-col-bases'><td><table class='adt-shadow'><tr></tr></table></td></tr>\
";

class ChainStep {

    constructor(node, data, step_no, dispatcher, options={}) {
        console.log("Chain.data", data, step_no);
        this.node = node;
        this.dispatcher = dispatcher;
        this.options = Object.assign({}, Chain_default_options, options);
        acmacs_web_title(acv_utils.format(Chain_acmacs_web_title_html, {id: data._id, min_col_basis: data.minimum_column_basis || "none"}), false);
        $("body .acmacs-web-header .acmacs-web-title .chain-id").on("click", evt => acv_toolkit.movable_window_with_json(data, evt.target, data.name || data.description || data._id));
        this.show(data, step_no);
    }

    show(data, step_no) {
        const table = $("<table></table>").appendTo(this.node);
        if (data.results[step_no])
            this.show_results(table, data.results[step_no], data.sources[step_no]);
        else
            this.show_sources(table, data.sources[step_no]);
    }

    show_results(table, results, source_id) {
        table.append(acv_utils.format(ChainStep_results_html, {id: source_id}));
        for (let [cell_type, tr_class] of [["i", "merge-incremental"], ["s", "merge-scratch"], ["1", "individual"], ["1m", "individual-col-bases"]]) {
            if (results[cell_type]) {
                const cell = this.make_map_cell(table.find(`tr.${tr_class} table tr`), cell_type);
                this.dispatcher.send_receive({C: "doc", id: results[cell_type]}, message => this.cell_map_add_content(cell, cell_type, message));
            }
        }
    }

    make_map_cell(node, cell_type) {
        return $(`<td><div class='map-cell-title'>${Chain_cell_type_to_name[cell_type]}<span class='chart-id ads-id-popup'></span></div></td>`).appendTo(node);
    }

    cell_map_add_content(cell, cell_type, message) {
        cell.find("span.chart-id").empty().append(message.doc._id).on("click", evt => acv_toolkit.movable_window_with_json(message.doc, evt.target, message.doc.name || message.doc.description || message.doc._id));
        if (message.doc.stresses && message.doc.stresses.length) {
            // cell.find("div").append(" " + message.doc.stresses[0].toFixed(2));
            antigenic_map_widget(cell, message.doc._id, this.dispatcher);
        }
    }

    show_sources(table, source_id) {
        $(acv_utils.format(ChainStep_source_html, {id: source_id})).appendTo(table);
    }
}

// ----------------------------------------------------------------------

function movable_window_with_json_for_id(id, invoking_node, dispatcher) {
    dispatcher.send_receive({C: "doc", id: id}, (message, dispatcher) => acv_toolkit.movable_window_with_json(message.doc, invoking_node, message.doc.name || message.doc.description || message.doc._id));
}

// ----------------------------------------------------------------------

function acmacs_web_title(text, replace=false) {
    let node = $("body .acmacs-web-header .acmacs-web-title");
    if (replace)
        node.empty();
    node.append(text);
}

// ----------------------------------------------------------------------

class AntigenicMapApi
{
    constructor(args) {         // {source_id, dispatcher}
        this.source_id = args.source_id;
        this.dispatcher = args.dispatcher;
    }

    download_pdf(args) {
        this._download({command: Object.assign({C: "pdf", id: this.source_id}, args), blob_type: "application/pdf"});
    }

    download_ace(args) {
        this._download({command: Object.assign({C: "download_ace", id: this.source_id}, args), blob_type: "application/octet-stream"});
    }

    download_save(args) {
        this._download({command: Object.assign({C: "download_lispmds_save", id: this.source_id}, args), blob_type: "application/octet-stream"});
    }

    download_layout_plain(args) {
        this._download({command: Object.assign({C: "download_layout_plain", id: this.source_id}, args), blob_type: "application/octet-stream"});
    }

    download_layout_csv(args) {
        this._download({command: Object.assign({C: "download_layout_csv", id: this.source_id}, args), blob_type: "application/octet-stream"});
    }

    download_table_map_distances_plain(args) {
        this._download({command: Object.assign({C: "download_table_map_distances_plain", id: this.source_id}, args), blob_type: "application/octet-stream"});
    }

    download_table_map_distances_csv(args) {
        this._download({command: Object.assign({C: "download_table_map_distances_csv", id: this.source_id}, args), blob_type: "application/octet-stream"});
    }

    download_error_lines(args) {
        this._download({command: Object.assign({C: "download_error_lines", id: this.source_id}, args), blob_type: "application/octet-stream"});
    }

    download_distances_between_all_points_plain(args) {
        this._download({command: Object.assign({C: "download_distances_between_all_points_plain", id: this.source_id}, args), blob_type: "application/octet-stream"});
    }

    download_distances_between_all_points_csv(args) {
        this._download({command: Object.assign({C: "download_distances_between_all_points_csv", id: this.source_id}, args), blob_type: "application/octet-stream"});
    }

    // {command:, blob_type:}
    _download(args) {
        this.dispatcher.send_receive(args.command, received => {
            const pdf = new window.Blob([received.data], {type: args.blob_type});
            const url = window.URL.createObjectURL(pdf);
            const link = $(`<a href='${url}' download='${received.header.name}'></a>`).appendTo($("body"));
            link[0].click();
            link.remove();
            window.setTimeout(() => {    // For Firefox it is necessary to delay revoking the ObjectURL
                window.URL.revokeObjectURL(url);
                // link.remove();
            }, 100);
        });
    }
}

// ----------------------------------------------------------------------

function antigenic_map_widget(parent, id, dispatcher) {
    const loader = async function() { return dispatcher.send_receive_async({C: "ace", id: id}); };
    import("../map-draw/ace-view-1/ace-view.js").then(mod => {
        const widget_options = {
            view_mode: "best-projection",
            coloring: "default",
            canvas_size: {width: 400, height: 400},
            title_fields: ["stress", "antigens", "sera", "date", "tables"],
            point_on_click: (point, invoking_node) => show_point_info(dispatcher, point, invoking_node),
            api: new AntigenicMapApi({source_id: id, dispatcher: dispatcher})
        };
        new mod.AntigenicMapWidget($("<div></div>").appendTo(parent), loader, widget_options);
    });
}

// function antigenic_map_widget(parent, id, dispatcher) {
//     dispatcher.send_receive({C: "ace", id: id}, message => {
//         import("../map-draw/ace-view-1/ace-view.js").then(mod => {
//             const widget_options = {
//                 view_mode: "best-projection",
//                 coloring: "default",
//                 canvas_size: {width: 400, height: 400},
//                 title_fields: ["stress", "antigens", "sera", "date", "tables"],
//                 point_on_click: show_point_info
//             };
//             new mod.AntigenicMapWidget($("<div></div>").appendTo(parent), message, widget_options);
//         });
//     });
// }

// ----------------------------------------------------------------------

function show_point_info(dispatcher, point, invoking_node) {

    const make_tables = tables => {
        if (!tables)
            return "";
        let by_lab = {};
        for (const tbl of tables) {
            const key = acv_utils.join_collapse([acv_utils.whocc_lab_name(tbl.lab), tbl.assay, tbl.rbc]);
            let ee = by_lab[key];
            if (ee)
                ee.push(tbl.date);
            else
                by_lab[key] = [tbl.date];
        }
        const make_for_lab = key => {
            const dates = by_lab[key].join("</li><li>");
            return `${key} (${by_lab[key].length})<ul class='a-tables-lab-date'><li>${dates}</li></ul>`;
        };
        const tbls = Object.keys(by_lab).sort().map(make_for_lab).join("</li><li>");
        return `<li class='a-tables'><ul class='a-tables-lab'><li>${tbls}</li><ul></li>`;
    };

    const make_window_antigen = (message, point, content) => {
        for (let antigen of message.antigens) {
            if (antigen.date) {
                content.append(`<div class='point-info-date'>Date: ${antigen.date}</div>`);
                break;
            }
        }

        const ul = $("<ul class='point-info-antigens point-info-antigens-sera'></ul>").appendTo(content);

        const make_row = (entry, expand=false) => {
            const name = acv_utils.join_collapse([entry.name, entry.reassortant, acv_utils.join_collapse(entry.annotations), entry.passage]);
            const lab_ids = entry.lab_ids ? "<li class='a-lab-ids'>" + acv_utils.join_collapse(entry.lab_ids, " ") +"</li>" : "";
            const data = `<ul class='point-info-data'>${lab_ids}${make_tables(entry.tables)}</ul>`;
            const li_class = expand ? "a-expanded" : "a-collapsed";
            const li = $(`<li class='${li_class}'><div class='a-expand a-icon'>&#9654;</div><div class='a-collapse a-icon'>&#9660;</div><div class='a-name'>${name}</div><div class='a-data'>${data}</div></li>`).appendTo(ul);
            li.find(".a-expand").on("click", evt => acv_utils.forward_event(evt, evt2 => li.addClass("a-expanded").removeClass("a-collapsed")));
            li.find(".a-collapse").on("click", evt => acv_utils.forward_event(evt, evt2 => li.addClass("a-collapsed").removeClass("a-expanded")));
        };

        const my_index = message.antigens.findIndex(elt => elt.reassortant === point.antigen.R && elt.passage === point.antigen.P && acv_utils.arrays_equal_simple(elt.annotations, point.antigen.a));
        if (my_index >= 0) {
            make_row(message.antigens[my_index], true);
            message.antigens.filter((entry, index) => index !== my_index).forEach(entry => make_row(entry));
        }
        else {
            message.antigens.forEach(make_row);
        }
    };

    const make_window_serum = (message, point, content) => {
        const ul = $("<ul class='point-info-sera point-info-antigens-sera'></ul>").appendTo(content);

        const make_row = entry => {
            const name = acv_utils.join_collapse([entry.name, entry.reassortant, acv_utils.join_collapse(entry.annotations), entry.serum_id]);
            const data = `<ul class='point-info-data'>${make_tables(entry.tables)}</ul>`;
            const li = $(`<li class='a-collapsed'><div class='a-expand a-icon'>&#9654;</div><div class='a-collapse a-icon'>&#9660;</div><div class='a-name'>${name}</div><div class='a-data'>${data}</div></li>`).appendTo(ul);
            li.find(".a-expand").on("click", evt => acv_utils.forward_event(evt, evt2 => li.addClass("a-expanded").removeClass("a-collapsed")));
            li.find(".a-collapse").on("click", evt => acv_utils.forward_event(evt, evt2 => li.addClass("a-collapsed").removeClass("a-expanded")));
        };

        const my_index = message.sera.findIndex(elt => elt.serum_id === point.serum.I);
        if (my_index >= 0) {
            make_row(message.sera[my_index], true);
            message.sera.filter((entry, index) => index !== my_index).forEach(entry => make_row(entry));
        }
        else {
            message.sera.forEach(make_row);
        }
    };

    const make_window = (message, invoking_node, title, point, content_filler) => {
        // console.log("show_point_info", message);
        const win = new acv_toolkit.MovableWindow({title: title, parent: invoking_node, classes: "hidb-point-info", content_css: {width: "auto", height: "30em"}});
        content_filler(message, point, win.content());
    };

    if (point.antigen) {
        const show = (message, dispatcher) => {
            const title = acv_utils.join_collapse(["AG", point.antigen.N, point.antigen.R, acv_utils.join_collapse(point.antigen.a), point.antigen.P]);
            return make_window(message, invoking_node, title, point, make_window_antigen);
        };
        dispatcher.send_receive({C: "hidb_antigen", name: point.antigen.N, passage: point.antigen.P, reassortant: point.antigen.R, annotations: point.antigen.a, lab_ids: point.antigen.l, virus_type: point.virus_type}, show);
    }
    else {
        const show = (message, dispatcher) => {
            const title = acv_utils.join_collapse(["SR", point.serum.N, point.serum.R, acv_utils.join_collapse(point.serum.a), point.serum.I]);
            return make_window(message, invoking_node, title, point, make_window_serum);
        };
        dispatcher.send_receive({C: "hidb_serum", name: point.serum.N, serum_id: point.serum.I, reassortant: point.serum.R, annotations: point.serum.a, virus_type: point.virus_type}, show);
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
