import { useEffect } from "react";
import * as v from "valibot";
import useWebSocket from "react-use-websocket";
import useStore from "@/stores/useStore";

const ConnectionSchema = v.object({
  connected: v.boolean(),
});

const ReportHidSchema = v.object({
  nid: v.number([v.integer()]),
  type: v.number([v.integer()]),
  hid: v.array(v.number([v.integer()]), [v.length(4)]),
});

const ReportNeighboursSchema = v.object({
  nid: v.number(),
  neighbours: v.array(v.number(), [v.length(4)]),
});

const SendDataSchema = v.object({
  sender: v.number(),
  data: v.number(),
});

const DisconnectSchema = v.object({
  removed_nid: v.number(),
});

export default function useSidecarEvents() {
  const { lastJsonMessage } = useWebSocket("ws://localhost:8080");

  const setConnected = useStore((state) => state.setConnected);
  const connectTile = useStore((state) => state.connectTile);
  const setNeighbours = useStore((state) => state.setNeighbours);
  const setData = useStore((state) => state.setData);
  const disconnectTile = useStore((state) => state.disconnectTile);

  useEffect(() => {
    // Core Connection Event
    if (v.safeParse(ConnectionSchema, lastJsonMessage).success) {
      const data = lastJsonMessage as v.Output<typeof ConnectionSchema>;
      setConnected(data.connected);
    }
    // Report Hardware Id Event
    else if (v.safeParse(ReportHidSchema, lastJsonMessage).success) {
      let data = lastJsonMessage as v.Output<typeof ReportHidSchema>;
      connectTile(data.nid, data.type, data.hid);
    }
    // Report Neighbours Event
    else if (v.safeParse(ReportNeighboursSchema, lastJsonMessage).success) {
      let data = lastJsonMessage as v.Output<typeof ReportNeighboursSchema>;
      setNeighbours(data.nid, data.neighbours);
    }
    // Send Data Event
    else if (v.safeParse(SendDataSchema, lastJsonMessage).success) {
      let data = lastJsonMessage as v.Output<typeof SendDataSchema>;
      setData(data.sender, data.data);
    }
    // Disconnect Event
    else if (v.safeParse(DisconnectSchema, lastJsonMessage).success) {
      let data = lastJsonMessage as v.Output<typeof DisconnectSchema>;
      disconnectTile(data.removed_nid);
    }
  }, [lastJsonMessage]);
}
