"use strict";

const initBox = document.getElementById("uninit");
const ipInput = document.getElementById("ip");
const portInput = document.getElementById("port");
const connectButton = document.getElementById("connect");
const statusSpan = document.getElementById("status");
const touchpad = document.getElementById("touchpad");

function showStatus(message) {
    console.warn(message);
    statusSpan.innerText = message;
    statusSpan.classList.remove("hidden");
}

function resetStatus() {
    statusSpan.innerText = "...";
    statusSpan.classList.add("hidden");
}

async function testConnection(server) {
    const endpoint = `${server}/test_connection`;
    try {
        const controller = new AbortController();
        const timeoutId = setTimeout(() => controller.abort(), 1000);
        const res = await fetch(endpoint, { signal: controller.signal });
        const text = await res.text();
        clearTimeout(timeoutId);
        return text === "Success";
    } catch (error) {
        return false;
    }
}

function goFullScreen() {
    // Go fullscreen on touchpad element
    if (touchpad.requestFullscreen) {
        touchpad.requestFullscreen();
    } else if (touchpad.mozRequestFullScreen) { // Firefox
        touchpad.mozRequestFullScreen();
    } else if (touchpad.webkitRequestFullscreen) { // Chrome, Safari and Opera
        touchpad.webkitRequestFullscreen();
    } else if (touchpad.msRequestFullscreen) { // IE/Edge
        touchpad.msRequestFullscreen();
    }

    document.addEventListener("fullscreenchange", () => {
        if (!document.fullscreenElement) {
            location.reload();
        }
    });
}

async function startTouchpad(ip, port) {
    const server = `http://${ip}:${port}`;

    // Test connection to test_connection endpoint
    const valid = await testConnection(server);

    if (!valid) {
        showStatus("Connection failed");
        return;
    }

    goFullScreen();

    uninit.style.display = "none";
    touchpad.style.display = "block";

    async function sendCoordinates(deltaX, deltaY, action) {
        const coords = `${deltaX},${deltaY},${action}`;
        const endpoint = `${server}/?data=${coords}`;
        try {
            const res = await fetch(endpoint, { method: "POST" });
            const text = await res.text();
            if (text !== "Success") {
                showStatus("Error sending coordinates");
            }
        } catch (error) {
            showStatus("Error sending coordinates");
        }
    }

    let isDragging = false;
    let lastX = null;
    let lastY = null;
    let touchStartTime = null;
    let longPressTimeout = null;
    const touchpadWidth = touchpad.clientWidth;
    const touchpadHeight = touchpad.clientHeight;

    touchpad.addEventListener("touchstart", (e) => {
        e.preventDefault();
        isDragging = true;
        const touch = e.touches[0];
        lastX = touch.clientX;
        lastY = touch.clientY;
        touchStartTime = Date.now();

        // Set up long press detection
        longPressTimeout = setTimeout(() => {
            if (isDragging) {
                sendCoordinates(0, 0, 2); // Long press action
                isDragging = false; // Prevent further drag actions
            }
        }, 500);
    });

    touchpad.addEventListener("touchmove", (e) => {
        if (!isDragging) return;
        e.preventDefault();
        clearTimeout(longPressTimeout); // Cancel long press if movement occurs
        const touch = e.touches[0];
        const currentX = touch.clientX;
        const currentY = touch.clientY;

        const deltaX = currentX - lastX;
        const deltaY = currentY - lastY;

        lastX = currentX;
        lastY = currentY;

        sendCoordinates(deltaY / touchpadHeight, -deltaX / touchpadWidth, 0); // Normal drag
    });

    touchpad.addEventListener("touchend", (e) => {
        e.preventDefault();
        clearTimeout(longPressTimeout); // Clear long press timeout
        const touchDuration = Date.now() - touchStartTime;

        if (isDragging && touchDuration < 100) {

            // If two fingers, send 2 instead
            if (e.touches.length > 1) {
                sendCoordinates(0, 0, 2); // Long press action
            } else {
                sendCoordinates(0, 0, 1); // Tap action
            }
        }

        isDragging = false;
        lastX = null;
        lastY = null;
    });
}

connectButton.addEventListener("click", (e) => {
    e.preventDefault();
    resetStatus();

    ip = ipInput.value;
    port = portInput.value;

    // Validate regex
    const ipRegex = /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
    if (!ipRegex.test(ip)) {
        showStatus("Invalid IP address.");
        return;
    }

    // Validate port range
    const portNumber = parseInt(port, 10);
    if (isNaN(portNumber) || portNumber < 1 || portNumber > 65535) {
        showStatus("Invalid port number. Must be between 1 and 65535.");
        return;
    }

    showStatus("Connecting...");
    startTouchpad(ip, portNumber);
});
