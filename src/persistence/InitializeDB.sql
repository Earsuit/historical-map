CREATE TABLE IF NOT EXISTS relationships (
    id INTEGER PRIMARY KEY,
    year_id INTEGER NOT NULL,
    country_id INTEGER NOT NULL,
    border_id INTEGER NOT NULL,
    FOREIGN KEY (country_id) REFERENCES countries(id),
    FOREIGN KEY (border_id) REFERENCES borders(id),
    FOREIGN KEY (year_id) REFERENCES years(id)
);

CREATE TABLE IF NOT EXISTS yearCities (
    id INTEGER PRIMARY KEY,
    city_id TEXT NOT NULL,
    year_id INTEGER NOT NULL,
    FOREIGN KEY (year_id) REFERENCES years(id)
    FOREIGN KEY (city_id) REFERENCES cities(id)
);

CREATE TABLE IF NOT EXISTS cities (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    latitude real NOT NULL,
    longitude real NOT NULL
);

CREATE TABLE IF NOT EXISTS events (
    id INTEGER PRIMARY KEY,
    year_id INTEGER NOT NULL,
    event TEXT NOT NULL,
    FOREIGN KEY (year_id) REFERENCES years(id)
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
    contour blob NOT NULL UNIQUE -- A list of (latitude, longitude) defines the contour of the country
);