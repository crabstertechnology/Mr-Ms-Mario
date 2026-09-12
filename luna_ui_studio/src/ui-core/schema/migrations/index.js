/**
 * Luna UI Studio — Schema Migration Infrastructure
 *
 * Provides extensible schema migrations so future updates to the canonical UI
 * model never break existing projects.
 */

import { CURRENT_SCHEMA_VERSION, parseSchemaVersion, compareVersions, isVersionSupported } from '../version.js'

/**
 * Migration registry: maps 'fromVersion->toVersion' to migration transform functions.
 * Future migrations will be registered here (e.g. '1.0.0->1.1.0': migrate_1_0_to_1_1).
 */
const MIGRATIONS = {}

/**
 * Registers a new schema migration step.
 * @param {string} fromVersion
 * @param {string} toVersion
 * @param {Function} migrationFn - Function transforming project from fromVersion to toVersion
 */
export function registerMigration(fromVersion, toVersion, migrationFn) {
  const key = `${fromVersion}->${toVersion}`
  MIGRATIONS[key] = migrationFn
}

/**
 * Migrates a project object to the target schema version.
 * If the project is already at the target version, it is returned directly.
 *
 * @param {Object} rawProject - The project object to migrate
 * @param {string} [targetVersion=CURRENT_SCHEMA_VERSION] - Target schema version
 * @returns {Object} Migrated project matching targetVersion
 */
export function migrateProject(rawProject, targetVersion = CURRENT_SCHEMA_VERSION) {
  if (!rawProject || typeof rawProject !== 'object') {
    throw new Error('Cannot migrate project: input must be a valid project object.')
  }

  const projectVersion = rawProject.version || '1.0.0'

  // If already at target version and version is supported, return project
  if (projectVersion === targetVersion) {
    if (!isVersionSupported(targetVersion)) {
      throw new Error(`Unsupported schema version: "${targetVersion}".`)
    }
    return rawProject
  }

  // Reject future versions that current runtime cannot understand
  if (compareVersions(projectVersion, targetVersion) > 0) {
    throw new Error(
      `Project schema version "${projectVersion}" is newer than current supported version "${targetVersion}". Please update Luna UI Studio.`
    )
  }

  // Check if a direct migration exists
  const directKey = `${projectVersion}->${targetVersion}`
  if (typeof MIGRATIONS[directKey] === 'function') {
    return MIGRATIONS[directKey](rawProject)
  }

  throw new Error(`No migration path found from schema version "${projectVersion}" to "${targetVersion}".`)
}
