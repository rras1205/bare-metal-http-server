const http = require('http');
const fs = require('fs');

// Load index.html into memory (same as C server)
const indexHtml = fs.readFileSync('index.html');

const server = http.createServer((req, res) => {
    if (req.method === 'GET' && req.url === '/') {
        res.writeHead(200, {
            'Content-Type': 'text/html',
            'Content-Length': indexHtml.length
        });
        res.end(indexHtml);
    } else {
        res.writeHead(404);
        res.end('Not Found');
    }
});

server.listen(3000, () => {
    console.log('Node.js server running on port 3000');
});
