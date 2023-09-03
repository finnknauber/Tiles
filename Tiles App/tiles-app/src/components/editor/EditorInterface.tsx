import useWebSocket from "react-use-websocket";
import Logs from "../debug/Logs";
import useStore from "@/stores/useStore";
import { ArrowPathRoundedSquareIcon } from "@heroicons/react/24/outline";

export default function EditorInterface() {
  const { sendMessage, lastMessage, lastJsonMessage } = useWebSocket(
    "ws://localhost:8080"
  );
  const connected = useStore((state) => state.connected);
  const rotation = useStore((state) => state.rotation);
  const rotate = useStore((state) => state.rotate);

  const onClick = async () => {
    sendMessage("update-config");
  };

  return (
    <>
      <span className="fixed top-4 left-4 z-10 flex flex-row gap-2 items-center leading-none bg-bg-600 px-2 py-1 text-sm rounded-full">
        <div
          className={`w-3 h-3 rounded-full shrink-0 ${connected ? "bg-emerald-300" : "bg-white bg-opacity-20"
            }`}
        ></div>
        {connected ? "Connected" : "Not Connected"}
      </span>
      {/* <section className='absolute transition-all border-l border-opacity-20 border-l-white right-0 top-0 h-full w-full max-w-4xl bg-bg-500 z-10'>
        <button onClick={onClick}>send stuff</button>
      </section> */}
      <button
        onClick={rotate}
        className="fixed top-4 flex items-center justify-center bg-bg-600 w-8 rounded-md z-10 h-8 right-4"
      >
        <ArrowPathRoundedSquareIcon
          className="w-4 h-4 transition-all"
          style={{ rotate: `${rotation}deg` }}
        />
      </button>
      <Logs />
    </>
  );
}
