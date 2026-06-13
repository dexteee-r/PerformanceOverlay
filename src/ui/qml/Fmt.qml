pragma Singleton
import QtQuick

// Helpers de formatage partagés par les écrans.
QtObject {
    function uptime(s) {
        const d = Math.floor(s / 86400)
        const h = Math.floor((s % 86400) / 3600)
        const m = Math.floor((s % 3600) / 60)
        return (d > 0 ? d + "j " : "") + ("0" + h).slice(-2) + ":" + ("0" + m).slice(-2)
    }
    function pad2(n) { return ("0" + n).slice(-2) }
    function countdown(min) {
        return min >= 60 ? Math.floor(min / 60) + "h " + pad2(min % 60) : min + " min"
    }
    function mem(mb) {
        return mb >= 1024 ? (mb / 1024).toFixed(1) + " Go" : Math.round(mb) + " Mo"
    }
    function rate(bps) {
        if (bps >= 1048576) return (bps / 1048576).toFixed(1) + " Mo/s"
        if (bps >= 1024) return Math.round(bps / 1024) + " Ko/s"
        return Math.round(bps) + " o/s"
    }
}
