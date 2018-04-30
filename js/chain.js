const sChainType = {
    "acmacs.inspectors.routine_diagnostics.IncrementalChainForked": "IncF",
    "acmacs.inspectors.routine_diagnostics.IncrementalChain": "Inc",
    "acmacs.inspectors.optimization.MapResolutionDeterminationByComputationalTiterPrediction": "MapRes",
    "acmacs.inspectors.routine_diagnostics.RoutineDiagnosticsChain": "RD",
    "acmacs.inspectors.optimization.OptimalMinimumColumnBasis": "OptMinColBasis",
    "acmacs.inspectors.optimization.DimensionTest": "Dim",
    "acmacs.inspectors.avidity.AvidityChain": "Avidity",
    "acmacs.inspectors.hemi_local.HemiGridChain": "Grid",
    "acmacs.inspectors.optimization.OptimizationExisting": "OptExisting"
};

const sVirusTypes = {"H1pdm": "H1", "A(H3N2)": "H3", "B/Vic": "B/Vic", "B/Yam": "B/Yam"};
const sVirusTypeOrder = ["H1", "H3", "B/Vic", "B/Yam", "Other"];

export function chains(data, dispatcher) {
    console.log(data, typeof(data.chains), data.chains);
    $("body").append(`<h3>Chains: ${data.chain_count}</h3>`);

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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
