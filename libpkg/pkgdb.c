#include <sys/param.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sqlite3.h>

#include "pkg.h"
#include "pkg_private.h"
#include "pkgdb.h"

#ifdef DEBUG
#include <dirent.h>
#include "pkg_compat.h"
#include "util.h"
#endif

#define PKG_DBDIR "/var/db/pkg"

static void pkgdb_stmt_to_pkg(sqlite3_stmt *, struct pkg *);
static void pkgdb_regex(sqlite3_context *, int, sqlite3_value **, int);
static void pkgdb_regex_basic(sqlite3_context *, int, sqlite3_value **);
static void pkgdb_regex_extended(sqlite3_context *, int, sqlite3_value **);
static void pkgdb_regex_delete(void *);

static void
pkgdb_regex_delete(void *ctx)
{
	regex_t *re = NULL;

	if ((re = (regex_t *)sqlite3_get_auxdata(ctx, 0)) != NULL) {
		regfree(re);
		free(re);
	}
}

static void
pkgdb_regex(sqlite3_context *ctx, int argc, sqlite3_value **argv, int reg_type)
{
	const char *regex = NULL;
	const char *str;
	regex_t *re;
	int ret;

	regex = sqlite3_value_text(argv[0]);
	str = sqlite3_value_text(argv[1]);

	if (argc != 2 || str == NULL || regex == NULL) {
		sqlite3_result_error(ctx, "SQL function regex() called with invalid arguments.\n", -1);
		return;
	}

	re = (regex_t *)sqlite3_get_auxdata(ctx, 0);
	if (re == NULL) {
		re = malloc(sizeof(regex_t));
		if (regcomp(re, regex, reg_type | REG_NOSUB) != 0) {
			sqlite3_result_error(ctx, "Invalid regex\n", -1);
			free(re);
			return;
		}

		sqlite3_set_auxdata(ctx, 0, re, pkgdb_regex_delete);
	}

	ret = regexec(re, str, 0, NULL, 0);
	sqlite3_result_int(ctx, (ret != REG_NOMATCH));
}

static void
pkgdb_regex_basic(sqlite3_context *ctx, int argc, sqlite3_value **argv)
{
	pkgdb_regex(ctx, argc, argv, REG_BASIC);
}

static void
pkgdb_regex_extended(sqlite3_context *ctx, int argc, sqlite3_value **argv)
{
	pkgdb_regex(ctx, argc, argv, REG_EXTENDED);
}

const char *
pkgdb_get_dir(void)
{
	const char *pkg_dbdir;

	if ((pkg_dbdir = getenv("PKG_DBDIR")) == NULL)
		pkg_dbdir = PKG_DBDIR;

	return pkg_dbdir;
}

static void
pkgdb_init(sqlite3 *sdb)
{
	char *errmsg;
	const char sql[] = ""
	"CREATE TABLE packages ("
		"origin TEXT PRIMARY KEY,"
		"name TEXT,"
		"version TEXT,"
		"comment TEXT,"
		"desc TEXT,"
		"automatic INTEGER"
	");"
	"CREATE TABLE options ("
		"package_id TEXT,"
		"name TEXT,"
		"with INTEGER,"
		"PRIMARY KEY (package_id,name)"
	");"
	"CREATE INDEX options_package ON options (package_id);"
	"CREATE TABLE deps ("
		"origin TEXT,"
		"name TEXT,"
		"version TEXT,"
		"package_id TEXT,"
		"PRIMARY KEY (package_id,origin)"
	");"
	"CREATE INDEX deps_origin ON deps (origin);"
	"CREATE INDEX deps_package ON deps (package_id);"
	"CREATE TABLE files ("
		"path TEXT PRIMARY KEY,"
		"md5 TEXT,"
		"package_id TEXT"
	");"
	"CREATE INDEX files_package ON files (package_id);"
	"CREATE TABLE conflicts ("
		"name TEXT,"
		"package_id TEXT,"
		"PRIMARY KEY (package_id,name)"
	");"
	"CREATE INDEX conflicts_package ON conflicts (package_id);";

	if (sqlite3_exec(sdb, sql, NULL, NULL, &errmsg) != SQLITE_OK)
		errx(EXIT_FAILURE, "sqlite3_exec(): %s", errmsg);

#ifdef DEBUG
	struct dirent **dirs;
	struct pkg_manifest *m;
	sqlite3_stmt *stmt_pkg;
	sqlite3_stmt *stmt_dep;
	sqlite3_stmt *stmt_conflicts;
	sqlite3_stmt *stmt_file;
	const char *dbdir;
	const char *conflict;
	char mpath[MAXPATHLEN];
	int nb_pkg;
	int i;

	dbdir = pkgdb_get_dir();
	nb_pkg = scandir(dbdir, &dirs, select_dir, alphasort);

	sqlite3_exec(sdb, "BEGIN TRANSACTION;", NULL, NULL, NULL);

	sqlite3_prepare(sdb, "INSERT INTO packages (origin, name, version, comment, desc)"
			"VALUES (?1, ?2, ?3, ?4, ?5);",
			-1, &stmt_pkg, NULL);

	sqlite3_prepare(sdb, "INSERT INTO deps (origin, name, version, package_id)"
			"VALUES (?1, ?2, ?3, ?4);",
			-1, &stmt_dep, NULL);

	sqlite3_prepare(sdb, "INSERT INTO conflicts (name, package_id)"
			"VALUES (?1, ?2, ?3, ?4);",
			-1, &stmt_conflicts, NULL);

	sqlite3_prepare(sdb, "INSERT INTO files (path, md5, package_id)"
			"VALUES (?1, ?2, ?3);",
			-1, &stmt_file, NULL);

	for (i = 0; i < nb_pkg; i++) {
		snprintf(mpath, sizeof(mpath), "%s/%s/+MANIFEST", dbdir, dirs[i]->d_name);
		if ((m = pkg_manifest_load_file(mpath)) == NULL &&
                    (m = pkg_compat_convert_installed(dbdir, dirs[i]->d_name, mpath)) == NULL) {
               continue;
          }

		sqlite3_bind_text(stmt_pkg, 1, pkg_manifest_value(m, "origin"), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt_pkg, 2, pkg_manifest_value(m, "name"), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt_pkg, 3, pkg_manifest_value(m, "version"), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt_pkg, 4, pkg_manifest_value(m, "comment"), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt_pkg, 5, pkg_manifest_value(m, "desc"), -1, SQLITE_STATIC);

		sqlite3_step(stmt_pkg);
		sqlite3_reset(stmt_pkg);

		pkg_manifest_dep_init(m);
		while (pkg_manifest_dep_next(m) == 0) {
			sqlite3_bind_text(stmt_dep, 1, pkg_manifest_dep_origin(m), -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt_dep, 2, pkg_manifest_dep_name(m), -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt_dep, 3, pkg_manifest_dep_version(m), -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt_dep, 4, pkg_manifest_value(m, "origin"), -1, SQLITE_STATIC);

			sqlite3_step(stmt_dep);
			sqlite3_reset(stmt_dep);
		}

		pkg_manifest_conflict_init(m);
		while ((conflict = pkg_manifest_conflict_next(m)) != NULL) {
			sqlite3_bind_text(stmt_conflicts, 1, conflict, -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt_conflicts, 2, pkg_manifest_value(m, "origin"), -1,
							  SQLITE_STATIC);

			sqlite3_step(stmt_conflicts);
			sqlite3_reset(stmt_conflicts);
		}

		pkg_manifest_file_init(m);
		while (pkg_manifest_file_next(m) == 0) {
			sqlite3_bind_text(stmt_file, 1, pkg_manifest_file_path(m), -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt_file, 2, pkg_manifest_file_md5(m), -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt_file, 3, pkg_manifest_value(m, "origin"), -1, SQLITE_STATIC);

			sqlite3_step(stmt_file);
			sqlite3_reset(stmt_file);
		}

		pkg_manifest_free(m);
		free(dirs[i]);
	}
	free(dirs);

	sqlite3_finalize(stmt_pkg);
	sqlite3_finalize(stmt_dep);
	sqlite3_finalize(stmt_conflicts);
	sqlite3_finalize(stmt_file);

	sqlite3_exec(sdb, "COMMIT;", NULL, NULL, NULL);
#endif
}

int
pkgdb_open(struct pkgdb **db)
{
	int retcode;
	struct stat st;
	char fpath[MAXPATHLEN];

	snprintf(fpath, sizeof(fpath), "%s/pkg.db", pkgdb_get_dir());

	if ((*db = malloc(sizeof(struct pkgdb))) == NULL)
		err(EXIT_FAILURE, "malloc()");

	if ((retcode = stat(fpath, &st)) == -1 && errno != ENOENT) {
		pkgdb_set_error(*db, errno, NULL);
		return (-1);
	}

	if (sqlite3_open(fpath, &(*db)->sqlite) != SQLITE_OK) {
		pkgdb_set_error(*db, 0, "sqlite3_open(): %s", sqlite3_errmsg((*db)->sqlite));
		return (-1);
	}

	if (retcode == -1)
		pkgdb_init((*db)->sqlite);

	sqlite3_create_function((*db)->sqlite, "regexp", 2, SQLITE_ANY, NULL,
							pkgdb_regex_basic, NULL, NULL);
	sqlite3_create_function((*db)->sqlite, "eregexp", 2, SQLITE_ANY, NULL,
							pkgdb_regex_extended, NULL, NULL);

	(*db)->stmt = NULL;
	(*db)->errnum = 0;
	(*db)->errstring[0] = '\0';

	return (0);
}

void
pkgdb_close(struct pkgdb *db)
{
	sqlite3_close(db->sqlite);
	free(db);
}

int
pkgdb_query_init(struct pkgdb *db, const char *pattern, match_t match)
{
	char sql[BUFSIZ];
	const char *comp = NULL;

	if (match != MATCH_ALL && pattern == NULL) {
		pkgdb_set_error(db, 0, "missing pattern");
		return (-1);
	}

	switch (match) {
	case MATCH_ALL:
		comp = "";
		break;
	case MATCH_EXACT:
		comp = " WHERE name = ?1";
		break;
	case MATCH_GLOB:
		comp = " WHERE name GLOB ?1";
		break;
	case MATCH_REGEX:
		comp = " WHERE name REGEXP ?1";
		break;
	case MATCH_EREGEX:
		comp = " WHERE EREGEXP(?1, name)";
		break;
	}

	snprintf(sql, sizeof(sql),
			"SELECT origin, name, version, comment, desc FROM packages%s;", comp);

	sqlite3_prepare(db->sqlite, sql, -1, &db->stmt, NULL);

	if (match != MATCH_ALL)
		sqlite3_bind_text(db->stmt, 1, pattern, -1, SQLITE_TRANSIENT);

	return (0);
}

void
pkgdb_query_free(struct pkgdb *db)
{
	sqlite3_finalize(db->stmt);
	db->stmt = NULL;
}

int
pkgdb_query(struct pkgdb *db, struct pkg *pkg)
{
	int retcode;

	pkg_reset(pkg);
	retcode = sqlite3_step(db->stmt);

	if (retcode == SQLITE_ROW) {
		pkgdb_stmt_to_pkg(db->stmt, pkg);
		pkg->pdb = db;
		return (0);
	} else if (retcode == SQLITE_DONE) {
		sqlite3_reset(db->stmt);
		return (1);
	} else {
		pkgdb_set_error(db, 0, "sqlite3_step(): %s", sqlite3_errmsg(db->sqlite));
		return (-1);
	}
}

int
pkgdb_query_which(struct pkgdb *db, const char *path, struct pkg *pkg)
{
	int retcode;

	pkg_reset(pkg);
	sqlite3_prepare(db->sqlite,
					"SELECT origin, name, version, comment, desc FROM packages, files "
					"WHERE origin = files.package_id "
					"AND files.path = ?1;", -1, &pkg->which_stmt, NULL);
	sqlite3_bind_text(pkg->which_stmt, 1, path, -1, SQLITE_STATIC);

	retcode = sqlite3_step(pkg->which_stmt);
	if (retcode == SQLITE_ROW) {
		pkgdb_stmt_to_pkg(pkg->which_stmt, pkg);
		pkg->pdb = db;
	}

	return ((retcode == SQLITE_ROW) ? 0 : 1);
}

int
pkgdb_query_dep(struct pkg *pkg, struct pkg *dep) {
	int retcode;

	if (pkg->deps_stmt == NULL) {
		sqlite3_prepare(pkg->pdb->sqlite,
						"SELECT p.origin, p.name, p.version, p.comment, p.desc FROM packages AS p, deps AS d "
						"WHERE p.origin = d.origin "
						"AND d.package_id = ?1;", -1, &pkg->deps_stmt, NULL);
		sqlite3_bind_text(pkg->deps_stmt, 1, pkg->origin, -1, SQLITE_STATIC);
	}

	retcode = sqlite3_step(pkg->deps_stmt);
	if (retcode == SQLITE_ROW) {
		pkgdb_stmt_to_pkg(pkg->deps_stmt, dep);
		dep->pdb = pkg->pdb;
		return (0);
	} else if (retcode == SQLITE_DONE) {
		sqlite3_reset(pkg->deps_stmt);
		return (1);
	} else {
		return (-1);
	}
}

int
pkgdb_query_rdep(struct pkg *pkg, struct pkg *rdep) {
	int retcode;

	if (pkg->rdeps_stmt == NULL) {
		sqlite3_prepare(pkg->pdb->sqlite,
						"SELECT p.origin, p.name, p.version, p.comment, p.desc FROM packages AS p, deps AS d "
						"WHERE p.origin = d.package_id "
						"AND d.origin = ?1;", -1, &pkg->rdeps_stmt, NULL);
		sqlite3_bind_text(pkg->rdeps_stmt, 1, pkg->origin, -1, SQLITE_STATIC);
	}

	retcode = sqlite3_step(pkg->rdeps_stmt);
	if (retcode == SQLITE_ROW) {
		pkgdb_stmt_to_pkg(pkg->rdeps_stmt, rdep);
		rdep->pdb = pkg->pdb;
		return (0);
	} else if (retcode == SQLITE_DONE) {
		sqlite3_reset(pkg->rdeps_stmt);
		return (1);
	} else {
		return (-1);
	}
}

int
pkgdb_query_conflicts(struct pkg *pkg, struct pkg *conflict) {
	int retcode;

	if (pkg->conflicts_stmt == NULL) {
		sqlite3_prepare(pkg->pdb->sqlite,
						"SELECT name, origin, version FROM conflicts "
						"WHERE package_id = ?1;", -1, &pkg->files_stmt, NULL);
		sqlite3_bind_text(pkg->files_stmt, 1, pkg->origin, -1, SQLITE_STATIC);
	}

	retcode = sqlite3_step(pkg->files_stmt);
	if (retcode == SQLITE_ROW) {
		conflict->name = strdup(sqlite3_column_text(pkg->files_stmt, 0));
		conflict->origin = strdup(sqlite3_column_text(pkg->files_stmt, 1));
		conflict->version = strdup(sqlite3_column_text(pkg->files_stmt, 2));
		return (0);
	} else if (retcode == SQLITE_DONE) {
		sqlite3_reset(pkg->files_stmt);
		return (1);
	} else {
		return (-1);
	}
}

int
pkgdb_query_files(struct pkg *pkg, const char **path, const char **md5) {
	int retcode;

	if (pkg->files_stmt == NULL) {
		sqlite3_prepare(pkg->pdb->sqlite,
						"SELECT path, md5 FROM files WHERE package_id = ?1;", -1, &pkg->files_stmt, NULL);
		sqlite3_bind_text(pkg->files_stmt, 1, pkg->origin, -1, SQLITE_STATIC);
	}

	retcode = sqlite3_step(pkg->files_stmt);
	if (retcode == SQLITE_ROW) {
		*path = sqlite3_column_text(pkg->files_stmt, 0);
		*md5 = sqlite3_column_text(pkg->files_stmt, 1);
		return (0);
	} else if (retcode == SQLITE_DONE) {
		sqlite3_reset(pkg->files_stmt);
		return (1);
	} else {
		return (-1);
	}
}

static void
pkgdb_stmt_to_pkg(sqlite3_stmt *stmt, struct pkg *pkg)
{
		pkg->origin = strdup(sqlite3_column_text(stmt, 0));
		pkg->name = strdup(sqlite3_column_text(stmt, 1));
		pkg->version = strdup(sqlite3_column_text(stmt, 2));
		pkg->comment = strdup(sqlite3_column_text(stmt, 3));
		pkg->desc = strdup(sqlite3_column_text(stmt, 4));
}

void
pkgdb_set_error(struct pkgdb *db, int errnum, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vsnprintf(db->errstring, sizeof(db->errstring), fmt, args);
	va_end(args);

	db->errnum = errnum;
}

void
pkgdb_warn(struct pkgdb *db)
{
	warnx("%s %s", db->errstring, (db->errnum > 0) ? strerror(db->errnum) : "");
}

int
pkgdb_errnum(struct pkgdb *db)
{
	return (db->errnum);
}
