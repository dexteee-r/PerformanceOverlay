import QtQuick
import QtQuick.Layouts
import PerformanceOverlay

// Vue cockpit (bento 3 colonnes). Le fond + le header sont fournis par AppShell.
Item {
    id: cockpit

    component KV: Item {
        property string k: ""
        property string v: ""
        property color vc: Theme.textHi
        implicitHeight: 30
        Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
               text: k; color: Theme.muted; font.family: Theme.fontUi; font.pixelSize: 12; font.letterSpacing: 1 }
        Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
               text: v; color: vc; font.family: Theme.fontMono; font.pixelSize: 14
               font.weight: Font.DemiBold; font.features: ({ "tnum": 1 }) }
        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.border2 }
    }
    component Readout: Column {
        property string big: ""
        property string unit: "%"
        property string sub: ""
        property color bigColor: Theme.textHi
        property int bigSize: 44
        spacing: 2
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 2
            Text { id: bigT; text: big; color: bigColor; font.family: Theme.fontMono
                   font.pixelSize: bigSize; font.weight: Font.Black; font.features: ({ "tnum": 1 }) }
            Text { text: unit; visible: unit !== ""; color: Theme.muted; font.family: Theme.fontMono
                   font.pixelSize: Math.round(bigSize * 0.36)
                   anchors.bottom: bigT.bottom; anchors.bottomMargin: Math.round(bigSize * 0.14) }
        }
        Text { anchors.horizontalCenter: parent.horizontalCenter; text: sub; visible: sub !== ""
               color: Theme.muted; font.family: Theme.fontUi; font.pixelSize: 10; font.letterSpacing: 3 }
    }

    RowLayout {
        anchors.fill: parent
        spacing: Theme.gap

        // ===================== COLONNE GAUCHE : CPU / GPU =====================
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 10
            spacing: Theme.gap

            // ----- CPU -----
            Panel {
                Layout.fillWidth: true; Layout.fillHeight: true
                title: "CPU"
                tag: Metrics.cpu.frequencyGhz.toFixed(2) + " GHz"
                statusColor: Theme.statusColor(Metrics.cpu.usagePercent / 100)
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10
                    Item {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        CircularGauge {
                            anchors.centerIn: parent
                            width: Math.min(parent.width, parent.height); height: width
                            lineWidth: 11
                            glow: true
                            value: Metrics.cpu.usagePercent / 100
                            arcColor: Theme.statusColor(Metrics.cpu.usagePercent / 100)
                            Readout {
                                anchors.centerIn: parent
                                big: Metrics.cpu.usagePercent.toFixed(0); sub: "CHARGE"
                                bigColor: Theme.statusColor(Metrics.cpu.usagePercent / 100)
                            }
                        }
                    }
                    Sparkline {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 34
                        values: Metrics.cpu.usageHistory
                        lineColor: Theme.statusColor(Metrics.cpu.usagePercent / 100)
                    }
                    StatBox { Layout.fillWidth: true; label: "FRÉQUENCE"; value: Metrics.cpu.frequencyGhz.toFixed(2); unit: "GHz" }
                }
            }

            // ----- GPU -----
            Panel {
                Layout.fillWidth: true; Layout.fillHeight: true
                title: "GPU"
                tag: Metrics.gpu.available ? "NVML" : ""
                alert: Metrics.gpu.available && Metrics.gpu.temperatureC >= 85
                statusColor: !Metrics.gpu.available ? Theme.muted
                             : Metrics.gpu.temperatureC >= 85 ? Theme.crit
                             : Metrics.gpu.temperatureC >= 72 ? Theme.warn : Theme.ok
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10
                    Item {
                        id: gpuGaugeArea
                        Layout.fillWidth: true; Layout.fillHeight: true
                        clip: true

                        Rectangle {                       // scanline GPU
                            visible: Metrics.gpu.available
                            width: parent.width; height: 64; y: -64; opacity: 0.7
                            gradient: Gradient {
                                GradientStop { position: 0.0; color: "transparent" }
                                GradientStop { position: 0.5; color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.10) }
                                GradientStop { position: 1.0; color: "transparent" }
                            }
                            SequentialAnimation on y {
                                loops: Animation.Infinite; running: true
                                NumberAnimation { from: -64; to: gpuGaugeArea.height; duration: 2400; easing.type: Easing.InOutQuad }
                                PauseAnimation { duration: 1400 }
                            }
                        }

                        CircularGauge {
                            anchors.centerIn: parent
                            visible: Metrics.gpu.available
                            width: Math.min(parent.width, parent.height); height: width
                            lineWidth: 11
                            glow: true
                            value: Metrics.gpu.usagePercent / 100
                            arcColor: Metrics.gpu.temperatureC >= 85 ? Theme.crit
                                      : Metrics.gpu.temperatureC >= 72 ? Theme.warn : Theme.accent
                            Readout { anchors.centerIn: parent; big: Metrics.gpu.usagePercent.toFixed(0); sub: "GPU" }
                        }
                        Text {
                            anchors.centerIn: parent; visible: !Metrics.gpu.available
                            text: Metrics.gpu.name; width: parent.width * 0.9
                            horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap
                            color: Theme.text; font.family: Theme.fontUi; font.pixelSize: 15
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true; spacing: 8; visible: Metrics.gpu.available
                        StatBox {
                            Layout.fillWidth: true; label: "TEMP"
                            value: Metrics.gpu.temperatureC.toFixed(0); unit: "°C"
                            valueColor: Metrics.gpu.temperatureC >= 85 ? Theme.crit
                                        : Metrics.gpu.temperatureC >= 72 ? Theme.warn : Theme.textHi
                        }
                        StatBox { Layout.fillWidth: true; label: "CONSO"; value: Metrics.gpu.powerW.toFixed(0); unit: "W" }
                        StatBox { Layout.fillWidth: true; label: "VRAM"; value: Metrics.gpu.vramUsedGb.toFixed(1); unit: "GB" }
                    }
                    Sparkline {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 30
                        visible: Metrics.gpu.available
                        values: Metrics.gpu.usageHistory
                        lineColor: Metrics.gpu.temperatureC >= 85 ? Theme.crit
                                   : Metrics.gpu.temperatureC >= 72 ? Theme.warn : Theme.accent
                    }
                }
            }
        }

        // ============ COLONNE CENTRE : NOYAU + STOCKAGE ============
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 14
            spacing: Theme.gap

            // ----- FLUX DE CHARGE (hero) -----
            Panel {
                Layout.fillWidth: true; Layout.fillHeight: true
                Layout.preferredHeight: 420
                title: "FLUX DE CHARGE"
                statusColor: Theme.statusColor(Metrics.systemLoad)
                tag: "CPU · GPU"
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8
                    CenterVisual {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        load: Metrics.systemLoad
                    }
                    Text { text: "FLUX SYSTÈME"; color: Theme.muted; font.family: Theme.fontUi
                           font.pixelSize: 9; font.letterSpacing: 3 }
                    Sparkline {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 48
                        values: Metrics.systemLoadHistory
                        lineColor: Theme.statusColor(Metrics.systemLoad)
                    }
                }
            }

            // ----- STOCKAGE -----
            Panel {
                Layout.fillWidth: true; Layout.fillHeight: true
                Layout.preferredHeight: 200
                title: "STOCKAGE"
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 14
                    Repeater {
                        model: Metrics.disk.disks
                        ColumnLayout {
                            required property var modelData
                            Layout.fillWidth: true
                            spacing: 6
                            RowLayout {
                                Layout.fillWidth: true
                                Text { text: "Disque " + modelData.name; color: Theme.text
                                       font.family: Theme.fontUi; font.pixelSize: 12; font.letterSpacing: 1 }
                                Item { Layout.fillWidth: true }
                                Text { text: Math.round(modelData.usagePercent) + " %"; color: Theme.textHi
                                       font.family: Theme.fontMono; font.pixelSize: 13; font.weight: Font.Bold
                                       font.features: ({ "tnum": 1 }) }
                            }
                            SegmentBar { Layout.fillWidth: true; segments: 34; value: modelData.usagePercent / 100 }
                        }
                    }
                    Item { Layout.fillHeight: true }
                }
            }
        }

        // ========= COLONNE DROITE : MÉMOIRE / SYSTÈME / PRIÈRE =========
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 10
            spacing: Theme.gap

            // ----- MÉMOIRE -----
            Panel {
                Layout.fillWidth: true; Layout.fillHeight: true
                title: "MÉMOIRE"
                statusColor: Theme.statusColor(Metrics.ram.usagePercent / 100)
                RowLayout {
                    anchors.fill: parent
                    spacing: 8
                    ColumnLayout {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        spacing: 4
                        Item {
                            Layout.fillWidth: true; Layout.fillHeight: true
                            CircularGauge {
                                anchors.centerIn: parent
                                width: Math.min(parent.width, parent.height, 96); height: width
                                lineWidth: 8
                                glow: true
                                value: Metrics.ram.usagePercent / 100
                                arcColor: Theme.statusColor(Metrics.ram.usagePercent / 100)
                                Readout { anchors.centerIn: parent; bigSize: 20; big: Metrics.ram.usagePercent.toFixed(0) }
                            }
                        }
                        Text { Layout.alignment: Qt.AlignHCenter; text: "RAM"; color: Theme.muted
                               font.family: Theme.fontUi; font.pixelSize: 10; font.letterSpacing: 2 }
                        Text { Layout.alignment: Qt.AlignHCenter
                               text: Metrics.ram.usedGb.toFixed(1) + "/" + Metrics.ram.totalGb.toFixed(0) + "G"
                               color: Theme.text; font.family: Theme.fontMono; font.pixelSize: 12; font.features: ({ "tnum": 1 }) }
                    }
                    ColumnLayout {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        spacing: 4
                        visible: Metrics.gpu.available
                        Item {
                            Layout.fillWidth: true; Layout.fillHeight: true
                            CircularGauge {
                                anchors.centerIn: parent
                                width: Math.min(parent.width, parent.height, 96); height: width
                                lineWidth: 8
                                glow: true
                                arcColor: Theme.accent2
                                value: Metrics.gpu.vramTotalGb > 0 ? Metrics.gpu.vramUsedGb / Metrics.gpu.vramTotalGb : 0
                                Readout { anchors.centerIn: parent; bigSize: 20
                                          big: Metrics.gpu.vramTotalGb > 0
                                               ? (Metrics.gpu.vramUsedGb / Metrics.gpu.vramTotalGb * 100).toFixed(0) : "0" }
                            }
                        }
                        Text { Layout.alignment: Qt.AlignHCenter; text: "VRAM"; color: Theme.muted
                               font.family: Theme.fontUi; font.pixelSize: 10; font.letterSpacing: 2 }
                        Text { Layout.alignment: Qt.AlignHCenter
                               text: Metrics.gpu.vramUsedGb.toFixed(1) + "/" + Metrics.gpu.vramTotalGb.toFixed(0) + "G"
                               color: Theme.text; font.family: Theme.fontMono; font.pixelSize: 12; font.features: ({ "tnum": 1 }) }
                    }
                }
            }

            // ----- SYSTÈME -----
            Panel {
                Layout.fillWidth: true; Layout.fillHeight: true
                title: "SYSTÈME"
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0
                    KV { Layout.fillWidth: true; k: "RÉSEAU"; v: Metrics.network.ipAddress
                         vc: Metrics.network.ipAddress === "No Network" ? Theme.crit : Theme.textHi }
                    KV { Layout.fillWidth: true; k: "VOLUME"
                         v: Metrics.volume.muted ? "MUTE" : Metrics.volume.level.toFixed(0) + " %"
                         vc: Metrics.volume.muted ? Theme.crit : Theme.textHi }
                    KV { Layout.fillWidth: true; k: "CLAUDE / JOUR"
                         v: Metrics.claudeUsage.hasData ? Metrics.claudeUsage.dayPercent.toFixed(0) + " %" : "--" }
                    KV { Layout.fillWidth: true; k: "CLAUDE / SEM."
                         v: Metrics.claudeUsage.hasData ? Metrics.claudeUsage.weekPercent.toFixed(0) + " %" : "--" }
                    Item {
                        Layout.fillWidth: true; Layout.topMargin: 8; implicitHeight: 6
                        Rectangle { anchors.fill: parent; color: Qt.rgba(1, 1, 1, 0.06) }
                        Rectangle {
                            height: parent.height
                            width: parent.width * Math.max(0, Math.min(1, Metrics.volume.level / 100))
                            color: Theme.accent2
                            Behavior on width { NumberAnimation { duration: 400; easing.type: Easing.OutCubic } }
                        }
                    }
                    Item { Layout.fillHeight: true }
                }
            }

            // ----- PRIÈRE -----
            Panel {
                Layout.fillWidth: true; Layout.fillHeight: true
                title: "PRIÈRE"
                accent: Theme.accent2
                statusColor: Theme.accent2
                tag: Metrics.prayer.usingApi ? "API" : "DÉFAUT"
                RowLayout {
                    anchors.fill: parent
                    ColumnLayout {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        spacing: 2
                        Text { text: "PROCHAINE"; color: Theme.muted; font.family: Theme.fontUi
                               font.pixelSize: 10; font.letterSpacing: 3 }
                        Text { text: Metrics.prayer.nextName; color: Theme.accent2; font.family: Theme.fontUi
                               font.pixelSize: 30; font.weight: Font.Black }
                        Text { text: Fmt.pad2(Metrics.prayer.nextHour) + ":" + Fmt.pad2(Metrics.prayer.nextMinute)
                               color: Theme.text; font.family: Theme.fontMono; font.pixelSize: 14; font.features: ({ "tnum": 1 }) }
                        Item { Layout.fillHeight: true }
                    }
                    ColumnLayout {
                        Layout.alignment: Qt.AlignRight | Qt.AlignBottom
                        spacing: 2
                        Text { Layout.alignment: Qt.AlignRight; text: "DANS"; color: Theme.muted
                               font.family: Theme.fontUi; font.pixelSize: 10; font.letterSpacing: 3 }
                        Text { Layout.alignment: Qt.AlignRight; text: Fmt.countdown(Metrics.prayer.remainingMinutes)
                               color: Theme.textHi; font.family: Theme.fontMono; font.pixelSize: 22; font.weight: Font.Bold
                               font.features: ({ "tnum": 1 }) }
                    }
                }
            }
        }
    }
}
