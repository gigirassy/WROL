FROM alpine:3.18

# Install build essentials and libcurl with OpenSSL (ensure proper SSL support)
RUN apk update && apk add --no-cache \
    build-base \
    curl-dev \
    libcurl \
    openssl-dev

WORKDIR /app
COPY . /app

# Build whitelist and application
RUN make all

EXPOSE 12000
CMD ["./bin/12ftpp"]
