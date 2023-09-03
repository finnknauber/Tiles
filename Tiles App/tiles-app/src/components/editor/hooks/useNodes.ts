import { useMemo } from "react";
import { Node } from "reactflow";
import BaseNode from "../Node";
import useStore from "@/stores/useStore";

export type CustomNode = Node & {
  data: {
    rotation: number;
  };
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
        data: {
          rotation: 0,
        },
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
      const lastNodeRotation = lastNode?.data.rotation || 0;
      const lastNodeNeighbours = nidToNeighbours[lastCurrentNID];
      const lastNodeNeighbourIndex = lastNodeNeighbours.findIndex(
        (nid) => nid === currentNID
      );
      const rotatedLastNeighbourIndex =
        lastNodeNeighbourIndex + ((lastNodeRotation / 90) % 4);

      const currentNodeNeighbours = nidToNeighbours[currentNID];
      const currentNodeNeighbourIndex = currentNodeNeighbours.findIndex(
        (nid) => nid === lastCurrentNID
      );

      const newNode: CustomNode = {
        id: currentNID.toString(),
        position: { x: 0, y: 0 },
        data: {
          rotation: 0,
        },
      };

      // set position: top, right, bottom, left
      switch (rotatedLastNeighbourIndex) {
        case 0: // top of lastNode (accounting for rotation)
          newNode.position = {
            x: lastNodePosition.x,
            y: lastNodePosition.y - nodeOffset,
          };
          switch (currentNodeNeighbourIndex) {
            case 0: // top
              newNode.data.rotation = 180;
              break;
            case 1: // right
              newNode.data.rotation = 90;
              break;
            case 2: // bottom
              newNode.data.rotation = 0;
              break;
            case 3: // left
              newNode.data.rotation = 270;
              break;
          }
          break;
        case 1: // right of lastNode (accounting for rotation)
          newNode.position = {
            x: lastNodePosition.x + nodeOffset,
            y: lastNodePosition.y,
          };
          switch (currentNodeNeighbourIndex) {
            case 0: // top
              newNode.data.rotation = 270;
              break;
            case 1: // right
              newNode.data.rotation = 180;
              break;
            case 2: // bottom
              newNode.data.rotation = 90;
              break;
            case 3: // left
              newNode.data.rotation = 0;
              break;
          }
          break;
        case 2: // bottom of lastNode (accounting for rotation)
          newNode.position = {
            x: lastNodePosition.x,
            y: lastNodePosition.y + nodeOffset,
          };
          switch (currentNodeNeighbourIndex) {
            case 0: // top
              newNode.data.rotation = 0;
              break;
            case 1: // right
              newNode.data.rotation = 270;
              break;
            case 2: // bottom
              newNode.data.rotation = 180;
              break;
            case 3: // left
              newNode.data.rotation = 90;
              break;
          }
          break;
        case 3: // left of lastNode (accounting for rotation)
          newNode.position = {
            x: lastNodePosition.x - nodeOffset,
            y: lastNodePosition.y,
          };
          switch (currentNodeNeighbourIndex) {
            case 0: // top
              newNode.data.rotation = 90;
              break;
            case 1: // right
              newNode.data.rotation = 0;
              break;
            case 2: // bottom
              newNode.data.rotation = 270;
              break;
            case 3: // left
              newNode.data.rotation = 180;
              break;
          }
          break;
      }

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
