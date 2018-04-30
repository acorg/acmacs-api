export function chains(data, dispatcher) {
    console.log(data, typeof(data.chains), data.chains);
    $("body").append(`<h3>Chains: ${data.chain_count}</h3>`);
    let ol = $("<ol></ol>").appendTo($("body"));
    data.chains.forEach(entry => {
        const text = JSON.stringify(entry, undefined, 2);
        ol.append(`<li style="border-top: 1px solid black"><pre style="margin-top: 0;">${text}</pre></li>\n`);
    });
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
