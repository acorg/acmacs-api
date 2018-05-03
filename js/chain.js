import {json_syntax_highlight, url_prefix} from "../draw/utils.js";
import {ADT_Popup1} from "../draw/toolkit.js";

export function chains(data, dispatcher) {
    let chains = new Chains($("<div class='chains'></div>").appendTo($("body")), data, dispatcher);
    $("head title").empty().append("AW: chains");
    $("body .acmacs-web-header .acmacs-web-title").empty().append("Acmacs-Web Chains");
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
        console.log("Chains.constructor", this.data);
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
        const span_name = `<a href="${url_prefix()}chain/${chain._id}" target="_blank" class='chains-chain-name'>${chain.name}</a>`;
        const span_id = `<span class='chains-chain-id'>${chain._id}</span>`;
        let classes = this.keyword_classes_(chain);
        const title = (chain.keywords && chain.keywords.length) ? "keywords: " + JSON.stringify(chain.keywords) : "";

        const chain_row = $(`<span class='${classes}' title='${title}'>${this.span_state_(chain)}${span_name}${this.span_modification_time_(chain)}${span_id}</span>`).appendTo(node);
        chain_row.find(".chains-chain-id").on("click", (evt) => { new ADT_Popup1(chain.name, `<pre class='json-highlight'>${json_syntax_highlight(JSON.stringify(chain, undefined, 2))}</pre>`, evt.target); });
        if (chain.forked_parent) {
            const sp = $("<br><span class='chains-chain-fork-of'><span class='chains-chain-fork-of-prefix'>fork of </span></span>").appendTo(chain_row);
            this.show_chain_(sp, chain.forked_parent);
            sp.append(`<span class='chains-chain-fork-of-suffix'> at ${chain.forked_step}</span>`);
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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
