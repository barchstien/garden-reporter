FROM python:3.10-slim

RUN pip install pipenv

ENV PROJECT_DIR /home/garden
RUN echo ${PROJECT_DIR}
RUN mkdir -p ${PROJECT_DIR}
RUN useradd garden -b ${PROJECT_DIR}
#RUN chown -R garden:garden ${PROJECT_DIR}

WORKDIR ${PROJECT_DIR}

COPY Pipfile Pipfile.lock collector.yaml ${PROJECT_DIR}/

RUN mkdir -p ${PROJECT_DIR}/.venv
RUN pipenv install --system --deploy --ignore-pipfile

RUN mkdir -p ${PROJECT_DIR}/source
COPY source/* ${PROJECT_DIR}/source/

RUN chown -R garden:garden ${PROJECT_DIR}
USER garden

#CMD ["/bin/bash -c pipenv run python3 /home/garden/source/collector.py"]
CMD ["python", "./source/collector.py"]

