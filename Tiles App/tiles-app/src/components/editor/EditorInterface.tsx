import { useEffect } from "react";
import useWebSocket, { ReadyState } from 'react-use-websocket';


export default function EditorInterface() {
    const selected = true

    const { readyState, sendMessage, lastMessage, lastJsonMessage } = useWebSocket("ws://localhost:8080");

    useEffect(() => {
        console.log(lastMessage?.data)
    }, [lastMessage, lastJsonMessage]);

    const onClick = async () => {
        sendMessage("toggle")
    }

    const connectionStatus = {
        [ReadyState.CONNECTING]: 'Connecting',
        [ReadyState.OPEN]: 'Open',
        [ReadyState.CLOSING]: 'Closing',
        [ReadyState.CLOSED]: 'Closed',
        [ReadyState.UNINSTANTIATED]: 'Uninstantiated',
    }[readyState];

    return (
        <section style={{
            transform: selected ? "translateX(0%)" : "translateX(100%)"
        }} className='absolute transition-all border-l border-opacity-20 border-l-white right-0 top-0 h-full w-full max-w-4xl bg-bg-500 z-10'>
            <span>{selected}</span>
            <span>{connectionStatus}</span>
            <br />
            <br />
            <button onClick={onClick}>send stuff</button>
        </section>
    )
}