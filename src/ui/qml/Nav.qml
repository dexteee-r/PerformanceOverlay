pragma Singleton
import QtQuick

// État de navigation global : la vue active.
//   "cockpit" | "compact" | "tasks" | "settings"
QtObject {
    property string view: "cockpit"
}
