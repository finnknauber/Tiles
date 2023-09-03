import { create } from "zustand";

interface AppState {
  connected: boolean;
  rotation: number;
  connectedNIDs: number[];
  nidToHid: Record<number, number[]>;
  nidToType: Record<number, number>;
  nidToNeighbours: Record<number, number[]>; // order: top, right, bottom, left
  nidToData: Record<number, number>;

  setConnected: (connected: boolean) => void;
  rotate: () => void;
  connectTile: (nid: number, type: number, hid: number[]) => void;
  setNeighbours: (nid: number, neighbours: number[]) => void;
  setData: (nid: number, data: number) => void;
  disconnectTile: (nid: number) => void;
}

const useStore = create<AppState>()((set, get) => ({
  connected: false,
  rotation: 0,
  connectedNIDs: [],
  nidToHid: {},
  nidToType: {},
  nidToNeighbours: {},
  nidToData: {},

  setConnected: (connected: boolean) => {
    set({ connected });
    if (!connected) {
      clearConnected();
    }
  },
  rotate: () => {
    const newRotation = (get().rotation + 90) % 360;
    set({ rotation: newRotation });
  },
  connectTile: (nid: number, type: number, hid: number[]) => {
    const newNidToHid = { ...get().nidToHid };
    newNidToHid[nid] = hid;
    const newNidToType = { ...get().nidToType };
    newNidToType[nid] = type;
    set({
      connectedNIDs: Array.from(new Set([...get().connectedNIDs, nid])),
      nidToHid: newNidToHid,
      nidToType: newNidToType,
    });
  },
  setNeighbours: (nid: number, neighbours: number[]) => {
    const newNidToNeighbours = { ...get().nidToNeighbours };
    newNidToNeighbours[nid] = neighbours;
    set({ nidToNeighbours: newNidToNeighbours });
  },
  setData: (nid: number, data: number) => {
    const newNidToData = { ...get().nidToData };
    newNidToData[nid] = data;
    set({ nidToData: newNidToData });
  },
  disconnectTile: (nid: number) => {
    const newConnectedNIDs = get().connectedNIDs.filter((n) => n !== nid);
    const newNidToHid = { ...get().nidToHid };
    delete newNidToHid[nid];
    const newNidToType = { ...get().nidToType };
    delete newNidToType[nid];
    const newNidToNeighbours = { ...get().nidToNeighbours };
    delete newNidToNeighbours[nid];
    const newNidToData = { ...get().nidToData };
    delete newNidToData[nid];
    set({
      connectedNIDs: newConnectedNIDs,
      nidToHid: newNidToHid,
      nidToType: newNidToType,
      nidToNeighbours: newNidToNeighbours,
      nidToData: newNidToData,
    });
  },
}));

/**
 * Resets all state related to the connected tiles
 */
function clearConnected() {
  useStore.setState({
    connectedNIDs: [],
    connected: false,
    nidToHid: {},
    nidToNeighbours: {},
    nidToType: {},
  });
}

export default useStore;
