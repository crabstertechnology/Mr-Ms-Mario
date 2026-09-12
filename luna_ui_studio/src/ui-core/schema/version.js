/**
 * Luna UI Studio — Schema Versioning Infrastructure
 *
 * Defines the current schema version and compatibility checking helpers.
 * Future schema changes will use this versioning system to ensure
 * backward compatibility for older projects.
 */

export const CURRENT_SCHEMA_VERSION = '1.0.0'

export const SUPPORTED_SCHEMA_VERSIONS = ['1.0.0']

/**
 * Parses a semver version string into major, minor, patch numbers.
 * @param {string} versionStr - e.g. "1.0.0"
 * @returns {{ major: number, minor: number, patch: number }}
 */
export function parseSchemaVersion(versionStr) {
  if (!versionStr || typeof versionStr !== 'string') {
    throw new Error(`Invalid schema version: "${versionStr}". Expected semver string (e.g. "1.0.0")`)
  }
  const parts = versionStr.split('.').map(Number)
  if (parts.length !== 3 || parts.some(isNaN)) {
    throw new Error(`Malformed schema version: "${versionStr}". Must follow semver (e.g. "1.0.0")`)
  }
  return { major: parts[0], minor: parts[1], patch: parts[2] }
}

/**
 * Checks if a project's schema version is supported by the current runtime.
 * @param {string} versionStr
 * @returns {boolean}
 */
export function isVersionSupported(versionStr) {
  return SUPPORTED_SCHEMA_VERSIONS.includes(versionStr)
}

/**
 * Compares two semver version strings.
 * @param {string} v1
 * @param {string} v2
 * @returns {number} -1 if v1 < v2, 0 if v1 == v2, 1 if v1 > v2
 */
export function compareVersions(v1, v2) {
  const p1 = parseSchemaVersion(v1)
  const p2 = parseSchemaVersion(v2)
  if (p1.major !== p2.major) return p1.major > p2.major ? 1 : -1
  if (p1.minor !== p2.minor) return p1.minor > p2.minor ? 1 : -1
  if (p1.patch !== p2.patch) return p1.patch > p2.patch ? 1 : -1
  return 0
}

/**
 * Identifies the schema version of a given project object.
 * Returns semver string (e.g. '1.0.0'), 'legacy', or 'unknown'.
 * @param {Object} project
 * @returns {string}
 */
export function identifySchemaVersion(project) {
  if (!project || typeof project !== 'object') return 'unknown'
  if (typeof project.version === 'string') return project.version
  if (Array.isArray(project.screens)) {
    // Check if it's legacy React Studio structure (elements with props)
    const hasLegacyElements = project.screens.some(
      (s) => Array.isArray(s.elements) && s.elements.some((el) => el && el.props)
    )
    if (hasLegacyElements) return 'legacy'
  }
  return 'unknown'
}

