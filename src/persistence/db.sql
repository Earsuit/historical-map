CREATE TABLE IF NOT EXISTS years (
    year INTEGER PRIMARY KEY,
    country_id INTEGER NOT NULL,
    border_id INTEGER NOT NULL,
    FOREIGN KEY (country_id) REFERENCES countries(id),
    FOREIGN KEY (border_id) REFERENCES borders(id)
);

CREATE TABLE IF NOT EXISTS countries (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS borders (
    id INTEGER PRIMARY KEY,
    contour blob NOT NULL  -- A list of (latitude, longitude) defines the contour of the country
);

CREATE TABLE IF NOT EXISTS cities (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    year INTEGER NOT NULL,
    country_id INTEGER NOT NULL,
    FOREIGN KEY (year) REFERENCES years(year),
    FOREIGN KEY (country_id) REFERENCES countries(id)
);

CREATE TABLE IF NOT EXISTS events (
    id INTEGER PRIMARY KEY,
    event TEXT NOT NULL,
    year INTEGER NOT NULL,
    FOREIGN KEY (year) REFERENCES years(year)
);