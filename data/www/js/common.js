// Common functions for the web interface

/*
    * @description: This function is used to get the current mode information from the server.
    * @returns: A promise that resolves to the mode information object.
    * @example: getModeInfo().then((data) => console.log(data));
    * data = {
    *     mode: "modeName",
    *     isRunning: true/false
    * }
*/
async function getModeInfo() {
    return {mode: "none", isRunning: false};
    const response = await fetch("/modeInfo");
    const data = await response.json();
    return data;
}

/*
    * @description: This function is used to get the current number of peers from the server.
    * @returns: A promise that resolves to the number of peers.
    * @example: getPeers().then((data) => console.log(data.peers+1));
    * NOTE : the server is not counting itself as a peer, so the number of pods is peers + 1
*/
async function getPeers() {
    return {peers: 0};
    const response = await fetch("/peers");
    const data = await response.json();
    return data.peers;
}
