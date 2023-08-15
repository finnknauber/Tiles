import { useMemo } from 'react';
import ReactFlow, { Background, BackgroundVariant, SelectionMode } from 'reactflow';
import EditorInterface from './components/editor/EditorInterface';
import Node from "@/components/editor/Node";

const initialNodes = [
	{ id: '1', position: { x: 0, y: 0 }, data: { label: '1' } },
	{ id: '2', position: { x: 0, y: 160 }, data: { label: '2' } },
	{ id: '3', position: { x: 160, y: 0 }, data: { label: '2' } },
	{ id: '4', position: { x: 160, y: 160 }, data: { label: '2' } },
	{ id: '5', position: { x: 160 * 2, y: 160 }, data: { label: '2' } },
	{ id: '6', position: { x: 160 * 2, y: 0 }, data: { label: '2' } },
];


function App() {
	const nodeTypes = useMemo(() => ({ default: Node }), []);

	return (
		<main className="w-[100vw] h-[100vh] bg-bg-500 text-white">
			<ReactFlow selectionMode={SelectionMode.Full} nodeTypes={nodeTypes} nodes={initialNodes} edges={[]} proOptions={{ hideAttribution: true }}>
				<Background color="rgba(255, 255, 255, .15)" gap={32} size={2} variant={BackgroundVariant.Dots} />
				<EditorInterface />
			</ReactFlow>
		</main >
	);
}

export default App;
