import {
  Background,
  BackgroundVariant,
  Node,
  ReactFlow,
  SelectionMode,
} from "reactflow";
import useNodes from "./hooks/useNodes";
import useWebSocket from "react-use-websocket";
import useStore from "@/stores/useStore";

type EditorProps = {
  children?: React.ReactNode;
};

export default function Editor({ children }: EditorProps) {
  const { nodes, nodeTypes } = useNodes();
  const nidToHid = useStore((state) => state.nidToHid);
  const { sendJsonMessage } = useWebSocket("ws://localhost:8080");

  const onNodeClick = (event: React.MouseEvent, node: Node) => {
    const nid = parseInt(node.id);
    sendJsonMessage({
      type: "node-select",
      hid: nidToHid[nid],
    });
  };

  return (
    <>
      <ReactFlow
        selectionMode={SelectionMode.Full}
        nodeTypes={nodeTypes}
        nodes={nodes}
        edges={[]}
        proOptions={{ hideAttribution: true }}
        onNodeClick={onNodeClick}
      >
        <Background
          color="rgba(255, 255, 255, .15)"
          gap={32}
          size={2}
          variant={BackgroundVariant.Dots}
        />
        {children}
      </ReactFlow>
    </>
  );
}
