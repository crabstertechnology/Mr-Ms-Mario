# Luna UI Core Architecture

The `ui-core` subsystem forms the **Canonical Architectural Foundation** for Luna UI Studio and its target embedded hardware runtimes.

---

## 1. Architectural Source of Truth & Boundary

> [!IMPORTANT]
> **Canonical UI Schema (`UIProject`) is the sole architectural source of truth for all UI layouts, styles, and interactions.**
> Generated C++ firmware and JSX preview trees are strictly downstream compilation artifacts.

### Current Migration State (Phase 1 / Phase 1.5 - Temporary Adapter Boundary):
```text
Legacy React Studio State (screens, elements, props)
                     ↓
      [ui-core/adapter/legacyAdapter.js]
                     ↓
            Canonical UIProject
```

> [!WARNING]
> **TEMPORARY ARCHITECTURAL ADAPTER:**
> The adapter pattern `Legacy Studio -> Adapter -> Canonical UIProject` is an intermediate migration step designed to prevent disrupting the existing React Canvas editor while establishing schema contracts.
> 
> **THE MANDATORY TARGET ARCHITECTURE:**
> ```text
> Studio Editor Canvas / State
>             ↓
>    Canonical UIProject (Schema)
>       ↙                     ↘
> React Web Renderer      Embedded Compiler
>                               ↓
>                        Embedded Runtime
> ```
> 
> **MANDATORY RULE:**
> No future embedded renderer or code compiler may consume the legacy `screens/elements` state representation directly. All compilation pipelines must target the Canonical `UIProject` schema.

---

## 2. Directory Layout

- `schema/`: Canonical schema definitions, semantic versioning (`version.js`), and migration infrastructure (`migrations/`).
- `device/`: Device profiles (`deviceProfiles.js`) specifying physical display parameters and conservative renderer capabilities contracts.
- `registry/`: Component definition contracts (`componentDefinition.js`), capability flags, supported states, events, and animations.
- `validation/`: Unsupported-feature reporting engine (`featureValidator.js`) emitting structured `ERROR`, `WARNING`, and `INFO` diagnostics.
- `adapter/`: Bidirectional bridge (`legacyAdapter.js`) between legacy Studio state and canonical `UIProject` with deterministic serialization.

---

## 3. Schema Versioning & Migrations

- Current Schema Version: `1.0.0`
- Extensible migrations are registered via `registerMigration(fromVersion, toVersion, fn)`.
- Automatic migration on load: `migrateProject(rawProject, targetVersion)`.
- Deterministic compilation: `serializeCanonicalProject(project, { stripTimestamps: true })` ensures byte-for-byte identical output for identical UI layouts.
