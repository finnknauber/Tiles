import { create } from 'zustand'

interface AppState {
    connected: boolean;
    connectedNIDs: number[];
    nidToHid: Record<number, number[]>;
    nidToType: Record<number, number>;
    nidToNeighbours: Record<number, number[]>; // order: top, right, bottom, left
    nidToData: Record<number, number>;

    setConnected: (connected: boolean) => void;
    connectTile: (nid: number, type: number, hid: number[]) => void;
    setNeighbours: (nid: number, neighbours: number[]) => void;
    setData: (nid: number, data: number) => void;
}

const useStore = create<AppState>()(
    (set, get) => ({
        connected: false,
        connectedNIDs: [],
        nidToHid: {},
        nidToType: {},
        nidToNeighbours: {},
        nidToData: {},

        setConnected: (connected: boolean) => {
            set({ connected })
            if (!connected) {
                clearConnected()
            }
        },
        connectTile: (nid: number, type: number, hid: number[]) => {
            const newNidToHid = { ...get().nidToHid }
            newNidToHid[nid] = hid
            const newNidToType = { ...get().nidToType }
            newNidToType[nid] = type
            set({
                connectedNIDs: Array.from(new Set([...get().connectedNIDs, nid])),
                nidToHid: newNidToHid,
                nidToType: newNidToType
            })
        },
        setNeighbours: (nid: number, neighbours: number[]) => {
            const newNidToNeighbours = { ...get().nidToNeighbours }
            newNidToNeighbours[nid] = neighbours
            set({ nidToNeighbours: newNidToNeighbours })
        },
        setData: (nid: number, data: number) => {
            const newNidToData = { ...get().nidToData }
            newNidToData[nid] = data
            set({ nidToData: newNidToData })
        }
    })
)

/**
 * Resets all state related to the connected tiles
 */
function clearConnected() {
    useStore.setState({
        connectedNIDs: [],
        connected: false,
        nidToHid: {},
        nidToNeighbours: {},
        nidToType: {}
    })
}

export default useStore