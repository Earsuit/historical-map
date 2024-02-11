CREATE TABLE IF NOT EXISTS relationships (
    id INTEGER PRIMARY KEY,
    year_id INTEGER NOT NULL,
    country_id INTEGER NOT NULL,
    border_id INTEGER NOT NULL,
    FOREIGN KEY (country_id) REFERENCES countries(id) ON DELETE CASCADE
    FOREIGN KEY (border_id) REFERENCES borders(id) ON DELETE CASCADE
    FOREIGN KEY (year_id) REFERENCES years(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS yearCities (
    id INTEGER PRIMARY KEY,
    city_id INTEGER NOT NULL,
    year_id INTEGER NOT NULL,
    FOREIGN KEY (year_id) REFERENCES years(id) ON DELETE CASCADE
    FOREIGN KEY (city_id) REFERENCES cities(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS cities (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL UNIQUE,
    latitude real NOT NULL,
    longitude real NOT NULL
);

CREATE TABLE IF NOT EXISTS yearNotes (
    id INTEGER PRIMARY KEY,
    year_id INTEGER NOT NULL UNIQUE,
    note_id INTEGER NOT NULL,
    FOREIGN KEY (year_id) REFERENCES years(id) ON DELETE CASCADE
    FOREIGN KEY (note_id) REFERENCES notes(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS notes (
    id INTEGER PRIMARY KEY,
    hash INTEGER NOT NULL UNIQUE,
    text TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS years (
    id INTEGER PRIMARY KEY,
    year INTEGER NOT NULL UNIQUE
);

CREATE TABLE IF NOT EXISTS countries (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL UNIQUE
);

CREATE TABLE IF NOT EXISTS borders (
    id INTEGER PRIMARY KEY,
    hash INTEGER NOT NULL UNIQUE,
    contour blob NOT NULL,
    CHECK ((hash IS NULL AND contour IS NULL) OR (hash IS NOT NULL AND contour IS NOT NULL))
);