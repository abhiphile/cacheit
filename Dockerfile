# Use an official C++ build image
FROM ubuntu:latest

# Install dependencies
RUN apt-get update && apt-get install -y g++ make cmake

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Build the server and client
RUN make

# Expose the server port
EXPOSE 6969

# Command to run the server by default
CMD ["./server"]
