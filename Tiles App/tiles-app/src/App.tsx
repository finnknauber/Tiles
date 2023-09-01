import EditorInterface from '@/components/editor/EditorInterface';
import useSidecarEvents from '@/components/hooks/useSidecarEvents';
import Editor from '@/components/editor/Editor';

function App() {
	useSidecarEvents()

	return (
		<main className="w-[100vw] h-[100vh] bg-bg-500 text-white">
			<Editor>
				<EditorInterface />
			</Editor>
		</main >
	);
}

export default App;
