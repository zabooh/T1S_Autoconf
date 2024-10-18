-- Definiere ein neues Protokoll mit einem eindeutigen Beschreibungstext
autoconf_proto = Proto("AutoconfMsgV3", "Autoconfiguration Message Protocol v3")

-- Definiere Felder für das Protokoll
local f_magic_code = ProtoField.uint32("autoconf.magic_code", "Magic Code", base.HEX)  -- Nur die ersten drei Argumente
local f_mac_addr = ProtoField.ether("autoconf.mac", "MAC Address")
local f_ip6_addr = ProtoField.ipv6("autoconf.ip6", "IPv6 Address")
local f_ip4_addr = ProtoField.ipv4("autoconf.ip4", "IPv4 Address")
local f_origin = ProtoField.uint8("autoconf.origin", "Origin", base.DEC)
local f_control_code = ProtoField.uint8("autoconf.control_code", "Control Code", base.DEC)
local f_node_max = ProtoField.uint8("autoconf.node_max", "Max Nodes", base.DEC)
local f_node_id = ProtoField.uint8("autoconf.node_id", "Node ID", base.DEC)
local f_randommssg = ProtoField.bytes("autoconf.randommssg", "Random Message", base.NONE)
local f_led_state = ProtoField.bool("autoconf.led_state", "LED State")
local f_random = ProtoField.uint32("autoconf.random", "Random", base.HEX)

-- Füge die Felder dem Protokoll hinzu
autoconf_proto.fields = { f_magic_code, f_mac_addr, f_ip6_addr, f_ip4_addr, f_origin, f_control_code, f_node_max, f_node_id, f_randommssg, f_led_state, f_random }

-- Diese Funktion wird aufgerufen, um Pakete zu dissektieren
function autoconf_proto.dissector(buffer, pinfo, tree)
    pinfo.cols.protocol = autoconf_proto.name

    -- Überprüfe, ob das Paket groß genug ist, um alle Felder zu enthalten
    if buffer:len() < 52 then
        return
    end

    -- Erstelle einen Unterbaum für das Protokoll
    local subtree = tree:add(autoconf_proto, buffer(), "Autoconfiguration Message")

    -- Dekodiere und zeige die Felder des Protokolls an (Magic Code und Counter als Little Endian)
    subtree:add_le(f_magic_code, buffer(0, 4))  -- Little Endian
    subtree:add(f_mac_addr, buffer(4, 6))
    subtree:add(f_ip6_addr, buffer(10, 16))
    subtree:add(f_ip4_addr, buffer(26, 4))
    subtree:add(f_origin, buffer(30, 1))
    subtree:add(f_control_code, buffer(31, 1))
    subtree:add(f_node_max, buffer(32, 1))
    subtree:add(f_node_id, buffer(33, 1))
    subtree:add(f_randommssg, buffer(34, 20))  -- Random Message (20 Bytes)

    -- Lese den Counter im 100ms Format (als Little Endian)
    local counter_100ms_value = buffer(54, 4):le_int()
    
    -- Berechne den Fließkommawert durch Multiplikation mit 0,1
    local counter_seconds = counter_100ms_value * 0.1

    -- Füge den ursprünglichen Counter hinzu (Little Endian)
    subtree:add_le(buffer(54, 4), string.format("Counter (100ms): %d", counter_100ms_value))

    -- Zeige den berechneten Fließkommawert an
    subtree:add(buffer(54, 4), string.format("Counter (Sekunden): %.1f", counter_seconds))

    subtree:add(f_led_state, buffer(58, 1))
    subtree:add(f_random, buffer(59, 4))
end

-- Registriere diesen Dissector für UDP-Pakete mit einem bestimmten Port (z.B. Port 47134)
udp_table = DissectorTable.get("udp.port")
udp_table:add(47134, autoconf_proto)
