import {ADT_Popup1} from "../draw/toolkit.js";

export function chains(data, dispatcher) {
    let chains = new Chains($("<div class='chains'></div>").appendTo($("body")), data, dispatcher);
    return;

    const virus_types = sVirusTypeOrder.reduce((target, vt) => { target[vt] = []; return target; }, {});
    data.chains.forEach((entry, index) => {
        if (entry.description)
            Object.entries(sVirusTypes).filter(kv => entry.description.indexOf(kv[0]) >= 0).forEach(kv => virus_types[kv[1]].push(index));
        else
            virus_types["Other"].push(index);
    });
    // console.log(virus_types);

    let vt_ul = $("<ul class='vt-list'></ul>").appendTo($("body"));
    for (let vt of sVirusTypeOrder) {
        if (virus_types[vt]) {
            let ol = $("<ol class='chain-list'></ol>").appendTo($(`<li>${virus_types[vt].length}</li>`).appendTo(vt_ul));
            for (let entry_index of virus_types[vt]) {
                let entry = data.chains[entry_index];
                let li = $("<li></li>").appendTo(ol);
                const chain_type = sChainType[entry["_t"]] || entry["_t"];
                let tables;
                if (!entry.sources || entry.sources.length <= 1)
                    tables = "";
                else if (entry.results === undefined || entry.results.length === entry.sources.length)
                    tables = `<span class='chain-tables'>T:${entry.sources.length}</span>`;
                else
                    tables = `<span class='chain-tables'>T:${entry.sources.length}(${entry.results.length})</span>`;
                li.append(`<span class='chain-type'>${chain_type}</span><span class='chain-name'>${entry.description || entry._id}</span>${tables}`);
            }
        }
    }

    // data.chains.forEach(entry => {
    //     let li = $("<li></li>").appendTo(ol);
    //     const chain_type = sChainType[entry["_t"]] || entry["_t"];
    //     let tables;
    //     if (!entry.sources || entry.sources.length <= 1)
    //         tables = "";
    //     else if (entry.results === undefined || entry.results.length === entry.sources.length)
    //         tables = `<span class='chain-tables'>T:${entry.sources.length}</span>`;
    //     else
    //         tables = `<span class='chain-tables'>T:${entry.sources.length}(${entry.results.length})</span>`;
    //     li.append(`<span class='chain-type'>${chain_type}</span><span class='chain-name'>${entry.description || entry._id}</span>${tables}`);
    // });

    // let ol = $("<ol></ol>").appendTo($("body"));
    // data.chains.forEach(entry => {
    //     const text = JSON.stringify(entry, undefined, 2);
    //     ol.append(`<li style="border-top: 1px solid black"><pre style="margin-top: 0;">${text}</pre></li>\n`);
    // });
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
        this.split_by_virus_type_lab(data);
        this.show();
    }

    show() {
        this.node.append(`<div class='chains-title'>Chains: ${this.chains_total}</div>`);
        let vt_ul = $("<ul class='chains-virus-types'></ul>").appendTo(this.node);
        for (let virus_type of sVirusTypeOrder)
            this.show_vt_(vt_ul, virus_type, this.data[virus_type]);
    }

    show_vt_(node, virus_type, entries) {
        if (entries !== undefined) {
            let vt_li = $(`<li><div class='chains-virus-type-name'>${virus_type}</div></li>`).appendTo(node);
            let lab_ul = $("<ul class='chains-labs'></ul>").appendTo(vt_li);
            for (let lab of sLabsOrder)
                this.show_lab_(lab_ul, lab, entries[lab]);
        }
    }

    show_lab_(node, lab, entries) {
        if (entries !== undefined) {
            let lab_li = $(`<li><div class='chains-lab-name'>${lab}</div></li>`).appendTo(node);
            let chain_ul = $("<ul class='chains-chains'></ul>").appendTo(lab_li);
            for (let chain of entries)
                this.show_chain_(chain_ul, chain);
        }
    }

    show_chain_(node, chain) {
        let classes = this.keyword_classes_(chain);
        let modif_time = "";
        if (chain._m) {
            let m_classes = ["chains-chain-m"];
            const oldness = new Date() - new Date(chain._m.substr(0, 10));
            if (oldness > (1000 * 60 * 60 * 24 * 183)) // 1/2 year
                m_classes.push("chains-chain-m-older-six-months");
            else if (oldness > (1000 * 60 * 60 * 24 * 30)) // 30 days
                m_classes.push("chains-chain-m-older-one-month");
            modif_time = `<span class='${m_classes.join(" ")}' title='${chain._m.substr(0, chain._m.indexOf("."))}'>[${chain._m.substr(0, 10)}]</span>`;
        }
        const title = (chain.keywords && chain.keywords.length) ? "keywords: " + JSON.stringify(chain.keywords) : "";
        let chain_li = $(`<li class='${classes}' title='${title}'><a href="/chain/${chain._id}" target="_blank" class='chains-chain-name'>${chain.name}</a>${modif_time}<span class='chains-chain-id'>${chain._id}</span></li>`).appendTo(node);
        chain_li.find(".chains-chain-id").on("click", (evt) => { new ADT_Popup1(chain._id, `<pre>${JSON.stringify(chain, undefined, 2)}</pre>`, evt.target); });
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
                classes.push("chains-chain-keyword-" + kw);
                break;
            default:
                console.warn("Unsupported chain keyword " + JSON.stringify(kw) + " of chain " + chain._id);
                break;
            }
        }
        return classes.join(" ");
    }

    split_by_virus_type_lab(data) {
        this.data = {};         // virus_type -> lab -> [fields]
        for (let chain_entry of data.chains) {
            const fields = this.find_fields(chain_entry);
            if (this.data[fields.virus_type] === undefined)
                this.data[fields.virus_type] = {};
            if (this.data[fields.virus_type][fields.lab] === undefined)
                this.data[fields.virus_type][fields.lab] = [];
            this.data[fields.virus_type][fields.lab].push(fields);
        }
        console.log("split_by_virus_type_lab", this.data);
    }

    find_fields(entry) {
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
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
