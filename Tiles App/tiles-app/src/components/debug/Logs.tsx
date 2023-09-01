import { useEffect, useState } from "react";
import useWebSocket, { ReadyState } from "react-use-websocket";

export default function Logs() {

    const [history, setHistory] = useState<string[]>([])
    const [showJson, setShowJson] = useState<boolean>(false)

    const { lastMessage, readyState } = useWebSocket("ws://localhost:8080");

    useEffect(() => {
        if (lastMessage) {
            const date = new Date()
            setHistory(curr => [`${date.getHours()}:${date.getMinutes()}:${date.getSeconds()}:${date.getMilliseconds()} ${lastMessage.data}`, ...curr])
        }
    }, [lastMessage]);

    const connectionStatus = {
        [ReadyState.CONNECTING]: 'Connecting',
        [ReadyState.OPEN]: 'Open',
        [ReadyState.CLOSING]: 'Closing',
        [ReadyState.CLOSED]: 'Closed',
        [ReadyState.UNINSTANTIATED]: 'Uninstantiated',
    }[readyState];

    return (
        <div className="flex z-20 flex-col w-full max-w-2xl max-h-80 fixed right-2 bottom-2 gap-1 p-4 bg-white bg-opacity-20 backdrop-blur">
            <span>Connection: {connectionStatus}</span>
            <div className="flex flex-row items-center gap-2">
                <input type="checkbox" id="showjson" checked={showJson} onChange={(e) => setShowJson(e.target.checked)} />
                <label htmlFor="showjson">Show Json</label>
            </div>
            <div className="flex flex-col gap-2 overflow-auto">
                {history.map((m, i) => {
                    if (m.endsWith("}") && !showJson) return
                    return <span className="text-sm font-mono" key={i}>{m}</span>
                })}
            </div>
        </div>
    )
}