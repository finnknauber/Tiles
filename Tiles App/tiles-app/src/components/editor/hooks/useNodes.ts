import { useMemo } from "react";
import { Node } from "reactflow";
import BaseNode from "../Node";
import useStore from "@/stores/useStore";

export type CustomNode = Node & {
  rotation: number;
};

const nodeOffset = 160;

export default function useNodes() {
  const nidToNeighbours = useStore((state) => state.nidToNeighbours);
  const rotation = useStore((state) => state.rotation);

  const nodeTypes = useMemo(() => ({ default: BaseNode }), []);

  const nodes = useMemo(() => {
    const coreNID = 1;
    let newNodes: CustomNode[] = [
      {
        id: coreNID.toString(),
        position: { x: 0, y: 0 },
        rotation: 0,
        data: {},
      },
    ];
    let lastCurrentNID = coreNID;
    let currentNID = parseInt(
      Object.keys(nidToNeighbours).find(
        (nid) =>
          parseInt(nid) !== coreNID &&
          nidToNeighbours[parseInt(nid)].includes(coreNID)
      ) || "0"
    );

    let completedNIDs: number[] = [];
    while (currentNID) {
      console.log(currentNID);
      // find position relative to last currentNID
      const lastNode = newNodes.find(
        (node) => node.id === lastCurrentNID.toString()
      );
      const lastNodePosition = lastNode?.position || { x: 0, y: 0 };
      const lastNodeRotation = lastNode?.rotation || 0;
      const lastNodeNeighbours = nidToNeighbours[lastCurrentNID];
      const lastNodeNeighbourIndex = lastNodeNeighbours.findIndex(
        (nid) => nid === currentNID
      );

      const currentNodeNeighbours = nidToNeighbours[currentNID];
      const currentNodeNeighbourIndex = currentNodeNeighbours.findIndex(
        (nid) => nid === lastCurrentNID
      );

      const newNode: CustomNode = {
        id: currentNID.toString(),
        position: { x: 0, y: 0 },
        rotation: rotation,
        data: {},
      };
      const rotatedIndex = (currentNodeNeighbourIndex + rotation / 90) % 4;
      // set position: top, right, bottom, left
      switch (rotatedIndex) {
        case 0: // top
          newNode.position = {
            x: lastNodePosition.x,
            y: lastNodePosition.y - nodeOffset,
          };
          break;
        case 1: // right
          newNode.position = {
            x: lastNodePosition.x + nodeOffset,
            y: lastNodePosition.y,
          };
          break;
        case 2: // bottom
          newNode.position = {
            x: lastNodePosition.x,
            y: lastNodePosition.y + nodeOffset,
          };
          break;
        case 3: // left
          newNode.position = {
            x: lastNodePosition.x - nodeOffset,
            y: lastNodePosition.y,
          };
          break;
      }
      // set rotation
      newNode.rotation = lastNodeRotation + currentNodeNeighbourIndex * 90;

      // mark as completed
      newNodes.push(newNode);
      completedNIDs.push(currentNID);
      lastCurrentNID = currentNID;

      // find next currentNID
      currentNID = parseInt(
        Object.keys(nidToNeighbours).find(
          (nid) =>
            // not coreNID
            parseInt(nid) !== coreNID &&
            // not completed
            !completedNIDs.includes(parseInt(nid)) &&
            // connected to a completedNID
            nidToNeighbours[parseInt(nid)].some((nid) =>
              completedNIDs.includes(nid)
            )
        ) || "0"
      );
    }

    console.log(newNodes);
    return newNodes;
  }, [nidToNeighbours, rotation]);

  return {
    nodes,
    nodeTypes,
  };
}
