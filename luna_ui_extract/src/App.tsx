import LunaDevice from "./luna/LunaDevice";
import ScreensExporter from "./ScreensExporter";

export default function App() {
  const isExport = window.location.search.includes("export");
  if (isExport) {
    return <ScreensExporter />;
  }

  return (
    <div className="font-dm size-full overflow-auto" style={{ background: "#0a0a14" }}>
      <LunaDevice />
    </div>
  );
}
