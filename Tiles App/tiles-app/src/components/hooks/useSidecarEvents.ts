import { useEffect } from "react";
import * as v from "valibot";
import useWebSocket from "react-use-websocket";
import useStore from "@/stores/useStore";
import { JsonValue } from "react-use-websocket/dist/lib/types";

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

export default function useSidecarEvents(useSocketEvents = true) {
  const { lastJsonMessage } = useWebSocket("ws://localhost:8080");

  const setConnected = useStore((state) => state.setConnected);
  const connectTile = useStore((state) => state.connectTile);
  const setNeighbours = useStore((state) => state.setNeighbours);
  const setData = useStore((state) => state.setData);
  const disconnectTile = useStore((state) => state.disconnectTile);

  const handleEvent = (event: JsonValue) => {
    // Core Connection Event
    if (v.safeParse(ConnectionSchema, event).success) {
      const data = event as v.Output<typeof ConnectionSchema>;
      setConnected(data.connected);
    }
    // Report Hardware Id Event
    else if (v.safeParse(ReportHidSchema, event).success) {
      let data = event as v.Output<typeof ReportHidSchema>;
      connectTile(data.nid, data.type, data.hid);
    }
    // Report Neighbours Event
    else if (v.safeParse(ReportNeighboursSchema, event).success) {
      let data = event as v.Output<typeof ReportNeighboursSchema>;
      setNeighbours(data.nid, data.neighbours);
    }
    // Send Data Event
    else if (v.safeParse(SendDataSchema, event).success) {
      let data = event as v.Output<typeof SendDataSchema>;
      setData(data.sender, data.data);
    }
    // Disconnect Event
    else if (v.safeParse(DisconnectSchema, event).success) {
      let data = event as v.Output<typeof DisconnectSchema>;
      disconnectTile(data.removed_nid);
    }
  };

  useEffect(() => {
    if (lastJsonMessage && useSocketEvents) {
      handleEvent(lastJsonMessage);
    }
  }, [lastJsonMessage, useSocketEvents]);

  return {
    handleEvent,
  };
}
