import useStore from "@/stores/useStore";
import { NodeProps } from "reactflow";

export default function Node({ id, selected, data }: NodeProps) {
  const nid = parseInt(id);
  const nidToHid = useStore((state) => state.nidToHid);
  const nidToType = useStore((state) => state.nidToType);
  const nidToData = useStore((state) => state.nidToData);

  return (
    <div
      style={{
        rotate: `${data?.rotation || 0}deg`,
      }}
      className={`bg-bg-500 border rounded-xl hover:bg-bg-600 transition-all w-32 h-32 flex flex-col gap-2 items-center justify-center text-white ${
        selected
          ? "border-white border-opacity-100 shadow-center-lg shadow-white"
          : "border-white border-opacity-20"
      }`}
    >
      <span className="text-xl font-semibold opacity-40">{id}</span>
      <span className="opacity-40">{nidToHid[nid]?.join(".")}</span>
      <span className="opacity-40">{data?.rotation || "none"}</span>
      <span className="opacity-40">
        {id === "1" ? "Core" : `Type ${nidToType[nid]}`}
      </span>
      <span className="opacity-40">{nidToData[nid]}</span>
    </div>
  );
}
