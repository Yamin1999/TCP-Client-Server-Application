-- Create the database (if it doesn't exist)
CREATE DATABASE IF NOT EXISTS user_db;

-- Use the database
USE user_db;

-- Create users table
CREATE TABLE IF NOT EXISTS users (
    user_id INT PRIMARY KEY,
    first_name VARCHAR(50) NOT NULL,
    last_name VARCHAR(50) NOT NULL,
    email VARCHAR(100) NOT NULL,
    city VARCHAR(50) NOT NULL
);

-- Create a dedicated user (optional, for better security)
-- CREATE USER IF NOT EXISTS 'app_user'@'%' IDENTIFIED BY 'app_password';
-- GRANT ALL PRIVILEGES ON user_db.* TO 'app_user'@'%';
-- FLUSH PRIVILEGES;