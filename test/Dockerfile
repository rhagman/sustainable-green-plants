FROM rust

WORKDIR /src

ARG WIKI_URL=https://github.com/rhagman/sustainable-green-plants.wiki.git

RUN cargo install marker
RUN git clone ${WIKI_URL}

ENTRYPOINT ["marker"]
