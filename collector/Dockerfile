FROM python:3.10-slim

ENV PROJECT_DIR /home/garden
RUN echo ${PROJECT_DIR}
RUN mkdir -p ${PROJECT_DIR}
RUN useradd garden -d ${PROJECT_DIR}

WORKDIR ${PROJECT_DIR}

RUN chown -R garden:garden ${PROJECT_DIR}
USER garden

COPY Pipfile Pipfile.lock collector.yaml ${PROJECT_DIR}/

RUN mkdir -p ${PROJECT_DIR}/source
COPY source/* ${PROJECT_DIR}/source/

RUN pip install pipenv
ENV PATH="${PATH}:${PROJECT_DIR}/.local/bin"

RUN mkdir -p ${PROJECT_DIR}/.venv
RUN pipenv install --system --deploy --ignore-pipfile

CMD ["python", "-u", "./source/collector.py"]

