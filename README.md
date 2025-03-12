# User Data Application

A client-server application for processing and storing user information. The client reads user data from a CSV file and sends it to the server, which stores it in a MySQL database.

## Features

- CSV file processing
- TCP socket communication
- MySQL database integration
- Logging capabilities
- Docker support for easy deployment
- CI/CD with GitHub Actions

## Project Structure

```
user-data-app/
├── src/                 # Source code
│   ├── client/          # Client application
│   └── server/          # Server application
├── include/             # Header files
├── build/               # Compiled binaries
├── scripts/             # Utility scripts and sample data
├── docker/              # Docker configuration
├── .github/workflows/   # CI/CD configuration
├── Makefile             # Build system
└── README.md            # Documentation
```

## Requirements

### For local development:
- GCC compiler
- Make build system
- MySQL client library (`libmysqlclient-dev`)

### For Docker deployment:
- Docker
- Docker Compose

## Building the Application

### Local build:

```bash
# Build both client and server
make all

# Build only the server
make server

# Build only the client
make client

# Clean build artifacts
make clean
```

### Docker build:

```bash
# Build and start all services
docker-compose -f docker/docker-compose.yml up -d

# Build and start only the server
docker-compose -f docker/docker-compose.yml up -d server

# Stop all services
docker-compose -f docker/docker-compose.yml down
```

## Usage

### Client Application:

```bash
./build/client <csv_file_path> <log_file_path>
```

Example:
```bash
./build/client scripts/test-data.csv logs/client.log
```

### Server Application:

```bash
./build/server
```

The server will listen on port 8080 for incoming connections.

## CSV File Format

The client expects a CSV file with the following columns:
- User ID (integer)
- First Name (string)
- Last Name (string)
- Email (string)
- City (string)

Example:
```
1,John,Doe,john.doe@gmail.com,Dhaka
2,Jane,Doe,jane.doe@gmail.com,Sylhet
```

## Database Setup

The server requires a MySQL database. You can set up the database using the provided SQL script:

```bash
mysql -u root -p < scripts/setup-db.sql
```

## Docker Setup

The application can be easily deployed using Docker Compose:

1. Make sure Docker and Docker Compose are installed
2. Run the application: `docker-compose -f docker/docker-compose.yml up -d`
3. Check logs: `docker logs user_server` or `docker logs user_client`
4. Stop the application: `docker-compose -f docker/docker-compose.yml down`

## Error Handling

The application implements comprehensive error handling:

- Socket creation and connection errors
- File I/O errors
- Database connection and query errors
- CSV parsing errors
- Graceful shutdown on signals

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/your-feature`)
3. Commit your changes (`git commit -am 'Add your feature'`)
4. Push to the branch (`git push origin feature/your-feature`)
5. Create a new Pull Request
