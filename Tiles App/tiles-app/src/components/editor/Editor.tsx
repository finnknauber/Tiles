import { Background, BackgroundVariant, ReactFlow, SelectionMode } from "reactflow";
import useNodes from "./hooks/useNodes";

type EditorProps = {
    children?: React.ReactNode;
}

export default function Editor({ children }: EditorProps) {
    const { nodes, nodeTypes } = useNodes()

    return (
        <>
            <ReactFlow
                selectionMode={SelectionMode.Full}
                nodeTypes={nodeTypes}
                nodes={nodes}
                edges={[]}
                proOptions={{ hideAttribution: true }}
            >
                <Background color="rgba(255, 255, 255, .15)" gap={32} size={2} variant={BackgroundVariant.Dots} />
                {children}
            </ReactFlow>
        </>
    )
}