import { NodeProps } from "reactflow"

export default function Node({ id, selected }: NodeProps) {
    return (
        <div className={`bg-bg-500 border rounded-xl hover:bg-bg-600 transition-all w-32 h-32 flex items-center justify-center text-white ${selected ? "border-indigo-800 border-opacity-100 shadow-center-lg shadow-indigo-900" : "border-white border-opacity-20"}`}>
            <span className="text-xl font-semibold opacity-40">{id}</span>
        </div>
    )
}