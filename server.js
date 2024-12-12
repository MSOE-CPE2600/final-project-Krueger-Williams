/********************************************************************
 *  Filename: server.js
 *  Class: CPE 2600 121
 *  Assignment Name: Lab 13 - Chat Application
 *  Description: Creates a proxy server for NodeJS forwarding TCP 
 *      connections between clients and servers
 *  Author: Krueger 'Mac' Williams & Chance Mason
 *  Date:  12/11/2024
 *  Note: run with: node server.js
 *******************************************************************/

const net = require('net');

const PROXY_PORT = 9090; // Port for client connections
const SERVER_PORT = 9091; // Port for server connections

let clients = [];
let serverSocket;

// Proxy server to handle connections from clients
const proxyServer = net.createServer((socket) => {
    clients.push(socket);
    console.log('Client connected');

    socket.on('data', (data) => {
        console.log(`Data received from client: ${data.toString()}`);
        // Forward data to the server
        if (serverSocket) {
            serverSocket.write(data);
        } else {
            console.log('No server connected');
        }
    });

    // Handle socket disconnect
    socket.on('end', () => {
        console.log('Client disconnected');
        clients = clients.filter(client => client !== socket);
    });

    // Handle socket error
    socket.on('error', (err) => {
        console.error('Socket error:', err.message);
    });
});

// Start listening for client connections
proxyServer.listen(PROXY_PORT, () => {
    console.log(`Proxy server listening on port ${PROXY_PORT}`);
});

// Server listener to handle outbound connections from servers
const serverListener = net.createServer((socket) => {
    serverSocket = socket;
    console.log('Server connected');

    // Socket received data
    socket.on('data', (data) => {
        console.log(`Data received from server: ${data.toString()}`);
        // Broadcast data to all clients
        clients.forEach((client) => {
            client.write(data);
        });
    });

    // Socket disconnected
    socket.on('end', () => {
        console.log('Server disconnected');
        serverSocket = null;
    });

    // Socket error
    socket.on('error', (err) => {
        console.error('Server socket error:', err.message);
    });
});

// Start listening for server connections
serverListener.listen(SERVER_PORT, () => {
    console.log(`Server listener listening on port ${SERVER_PORT}`);
});








