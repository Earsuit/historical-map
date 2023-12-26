CREATE TABLE years (
    year INTEGER PRIMARY KEY,
    country_id INTEGER NOT NULL,
    border_id INTEGER NOT NULL,
    FOREIGN KEY (country_id) REFERENCES countries(id),
    FOREIGN KEY (border_id) REFERENCES borders(id)
);

CREATE TABLE countries (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL
);

CREATE TABLE borders (
    id INTEGER PRIMARY KEY,
    contour blob NOT NULL  -- A list of (latitude, longitude) defines the contour of the country
);

CREATE TABLE cities (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    year INTEGER NOT NULL,
    country_id INTEGER NOT NULL,
    FOREIGN KEY (year) REFERENCES years(year),
    FOREIGN KEY (country_id) REFERENCES countries(id)
);

CREATE TABLE events (
    id INTEGER PRIMARY KEY,
    event TEXT NOT NULL,
    year INTEGER NOT NULL,
    FOREIGN KEY (year) REFERENCES years(year)
);