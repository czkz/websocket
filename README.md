# Websockets
Websocket client/server implementation on top of [my socket library](https://github.com/czkz/socks).

## Usage
1. Build the project:
   ```bash
   meson build
   ninja -C build
   ```
2. Run the server:
   ```bash
   ./build/websock
   ```
3. Open an http webpage with CORS disabled (for example, using node [http-server](https://github.com/http-party/http-server))
4. Open DevTools\-\>Console
5. Send something to the server:
   ```javascript
   var s = new WebSocket('ws://127.0.0.1:1337');
   s.onmessage = msg => console.log(msg.data);
   s.onopen = () => { s.send('Hello, World!'); }
   ```
