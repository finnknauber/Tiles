import { useMemo } from "react"
import { Node } from "reactflow"
import BaseNode from "../Node";
import useStore from "@/stores/useStore";

export default function useNodes() {

    const nidToNeighbours = useStore(state => state.nidToNeighbours)

    const nodeTypes = useMemo(() => ({ default: BaseNode }), []);

    const nodes = useMemo(() => {
        const coreNID = 1
        let newNodes: Node[] = [{
            id: coreNID.toString(),
            position: { x: 0, y: 0 },
            data: {}
        }]
        let currentNID = parseInt(Object.keys(nidToNeighbours).find(nid => parseInt(nid) !== coreNID && nidToNeighbours[parseInt(nid)].includes(coreNID)) || "0")

        let completedNIDs: number[] = []
        let nextNIDs: number[] = nidToNeighbours[currentNID]?.filter(nid => nid !== coreNID && nid !== 0) || []
        while (currentNID) {
            // find next nids to work through
            const connectedNIDs = nidToNeighbours[currentNID]?.filter(nid => nid !== coreNID && nid !== 0 && !completedNIDs.includes(nid))
            nextNIDs = nextNIDs.concat(connectedNIDs)

            // add node
            const relatedNode = newNodes.find(n => nidToNeighbours[currentNID]?.includes(parseInt(n.id)))
            const relatedNodeIdx = relatedNode ? nidToNeighbours[parseInt(relatedNode.id)].indexOf(currentNID) : 0
            const distance = 160
            newNodes.push({
                id: currentNID.toString(),
                position: relatedNode ?
                    relatedNodeIdx === 0 ? // top
                        { x: relatedNode.position.x, y: relatedNode.position.y - distance } :
                        relatedNodeIdx === 1 ? // right
                            { x: relatedNode.position.x + distance, y: relatedNode.position.y } :
                            relatedNodeIdx === 2 ? // bottom
                                { x: relatedNode.position.x, y: relatedNode.position.y + distance } :
                                // left
                                { x: relatedNode.position.x - distance, y: relatedNode.position.y }
                    : { x: 0, y: 0 },
                data: {}
            })

            // mark completed nid and get next nid
            completedNIDs.push(currentNID)
            currentNID = nextNIDs.length ? nextNIDs.splice(0, 1)[0] : 0
        }

        console.log(newNodes)
        return newNodes
    }, [nidToNeighbours])

    return {
        nodes,
        nodeTypes
    }
}